#include "tcp/server.hpp"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "tcp/error.hpp"

namespace tcp {

Server::~Server() { stop(true); }

Server& Server::set_port(unsigned int port_no) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set port while server is running");
  }
  this->port_no = port_no;
  return *this;
}

Server& Server::set_ip_addr(const char* ip_addr) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set IP address while server is running");
  }
  // copy the IP address (map to IPv6 if needed)
  bool is_ipv6 = false;
  for (auto p = ip_addr; *p; p++) {
    if (*p == ':') {
      is_ipv6 = true;
      break;
    }
  }
  if (!is_ipv6) {
    snprintf(server_ip_addr, INET6_ADDRSTRLEN, "::ffff:%s", ip_addr);
  } else {
    strncpy(server_ip_addr, ip_addr, INET6_ADDRSTRLEN);
    server_ip_addr[INET6_ADDRSTRLEN - 1] = '\0';
  }
  return *this;
}

Server& Server::set_timeout(unsigned int timeout) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set timeout while server is running");
  }
  this->timeout = timeout;
  return *this;
}

Server& Server::set_max_timeouts(unsigned int max_timeouts) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set max timeouts while server is running");
  }
  this->max_timeouts = max_timeouts;
  return *this;
}

Server& Server::set_backlog(unsigned int backlog) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set backlog while server is running");
  }
  this->backlog = backlog;
  return *this;
}

Server& Server::debug(bool mode) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set debug mode while server is running");
  }
  this->debug_mode = mode;
  client_handler.debug(mode);
  return *this;
}

Server& Server::set_timeout_handler(TimeoutFunction handler) {
  if (server_pid >= 0) {
    throw ConfigurationError(
        "Cannot set timeout handler while server is running");
  }
  this->timeout_handler = handler;
  return *this;
}

Server& Server::use_threads() {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set use threads while server is running");
  }
  std::cerr << "WARNING: Using threads is very new and may not work as expected"
            << std::endl;
  std::cerr << "WARNING: max_clients is not supported with threads"
            << std::endl;

  client_handler.use_threads();
  use_thread = true;
  return *this;
}

Server& Server::add_handler(ClientHandlerFunction handler) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot add handler while server is running");
  }
  client_handler.add_handler(handler);
  return *this;
}

Server& Server::set_handler_mode(ClientHandler::handle_mode mode) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set handler mode while server is running");
  }
  client_handler.set_mode(mode);
  return *this;
}

Server& Server::set_max_clients(unsigned int max_clients) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set max clients while server is running");
  }
  client_handler.max_clients = max_clients;
  return *this;
}

Server& Server::add_handler_extra_data(void* data) {
  if (server_pid >= 0) {
    throw ConfigurationError("Cannot set extra data while server is running");
  }
  client_handler.set_extra_data(data);
  return *this;
}

pid_t Server::start() {
  // verify configuration
  if (server_pid >= 0) {
    throw ConfigurationError("Server already running");
  }
  if (port_no == 0) {
    throw ConfigurationError("Port number not set");
  }
  if (backlog == 0) {
    throw ConfigurationError("Backlog not set");
  }
  if (client_handler.handlers.size() == 0) {
    throw ConfigurationError("No client handlers set");
  }
  if (client_handler.max_clients == 0) {
    throw ConfigurationError("Max clients not set");
  }

  if (!use_thread) {
    server_pid = fork();

    if (server_pid < 0) {
      perror("TCPServer fork");
      return server_pid;
    }

    if (server_pid == 0) {
      run_server();
      exit(EXIT_SUCCESS);
    }
  } else {
    std::thread server_thread([this]() { run_server(); });
    server_thread.detach();
  }

  return server_pid;
}

void Server::exec() {
  auto pid = start();
  if (use_thread) {
    while (true) {
      sleep(1);
    }
  } else {
    if (pid < 0) {
      perror("TCPServer exec");
      exit(EXIT_FAILURE);
    }

    if (waitpid(pid, nullptr, 0) < 0) {
      perror("TCPServer waitpid");
    }
  }
  exit(EXIT_SUCCESS);
}

void Server::run_server() {
  if (debug_mode) {
    fprintf(stderr, "Starting server process pid: %d\n", getpid());
  }

  // create a socket file descriptor for the server
  server_sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (server_sock_fd < 0) {
    perror("TCPServer socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in6 server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port_no);
  server_addr.sin6_addr = in6addr_any;  // default to any address
  if (*server_ip_addr != '\0') {        // if server IP address is provided
    int success = inet_pton(AF_INET6, server_ip_addr, &server_addr.sin6_addr);
    if (success <= 0) {
      if (success == 0) {
        fprintf(stderr, "Invalid IP Address: %s\n", server_ip_addr);
      } else {
        perror("TCPServer inet_pton");
      }
      close(server_sock_fd);
      exit(EXIT_FAILURE);
    }
  }

  if (bind(server_sock_fd, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("TCPServer bind");
    close(server_sock_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_sock_fd, backlog) < 0) {
    perror("TCPServer listen");
    close(server_sock_fd);
    exit(EXIT_FAILURE);
  }

  if (debug_mode) {
    fprintf(stderr, "TCPServer started on port %d\n", port_no);
  }

  while (true) {
    client_handler.reap_clients();

    if (timeout > 0) {
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(server_sock_fd, &read_fds);

      struct timeval tv;
      tv.tv_sec = timeout;
      tv.tv_usec = 0;

      if (debug_mode) {
        fprintf(stderr, "Waiting for up to %ds for a new connection\n",
                timeout);
      }

      int ret = select(server_sock_fd + 1, &read_fds, NULL, NULL, &tv);
      if (ret < 0) {
        perror("TCPServer select");
        if (errno == EINTR) {
          continue;
        }
        // stop server on error
        break;
      }

      if (ret == 0) {
        if (debug_mode) {
          fprintf(stderr, "TCPServer timeout\n");
        }

        timeout_count++;
        // call timeout handler
        if (timeout_handler != nullptr) {
          if (debug_mode) {
            fprintf(stderr, "Calling timeout handler\n");
          }
          timeout_handler();
        }
        if (max_timeouts > 0 && timeout_count >= max_timeouts) {
          if (debug_mode) {
            fprintf(stderr, "Max timeouts reached\n");
          }
          // stop server on max timeouts
          break;
        }
        // continue waiting for connections
        continue;
      }
    }

    // socket is ready to accept a new connection
    timeout_count = 0;

    // pass client to handler
    client_handler.accept(server_sock_fd);
  }

  if (debug_mode) {
    fprintf(stderr, "Stopping server thread\n");
  }

  close(server_sock_fd);

  client_handler.join_clients();
}

void Server::stop(bool force) {
  if (server_pid < 0) {
    return;
  }

  if (force) {
    if (kill(server_pid, SIGKILL) < 0) {
      perror("TCPServer kill");
    }
  } else {
    if (kill(server_pid, SIGTERM) < 0) {
      perror("TCPServer kill");
    }
  }

  if (waitpid(server_pid, nullptr, 0) < 0) {
    perror("TCPServer waitpid");
  }

  server_pid = -1;
}

// reap all clients that have finished
void Server::ClientHandler::reap_clients() {
  if (debug_mode) {
    fprintf(stderr, "Reaping clients\n");
  }

  if (clients.empty()) {
    return;
  }

  int reap_count = 0;
  pid_t finished;
  while (!clients.empty() && (finished = waitpid(0, nullptr, WNOHANG))) {
    if (finished < 0) {
      perror("TCPServer waitpid");
      break;
    }
    clients.erase(std::remove(clients.begin(), clients.end(), finished),
                  clients.end());
    reap_count++;
  }

  if (debug_mode) {
    fprintf(stderr, "Reaped %d clients\n", reap_count);
  }

  if (debug_mode) {
    fprintf(stderr, "%lu connected clients remaining\n", clients.size());
  }
}

// wait for all clients to finish
void Server::ClientHandler::join_clients() {
  if (debug_mode) {
    fprintf(stderr, "Joining clients\n");
  }

  for (unsigned int i = 0; i < clients.size(); i++) {
    if (waitpid(clients[i], nullptr, 0) < 0) {
      perror("TCPServer waitpid");
    }
  }
  clients.clear();
}

// tell all clients to terminate
void Server::ClientHandler::terminate_clients() {
  if (debug_mode) {
    fprintf(stderr, "Terminating clients\n");
  }

  for (unsigned int i = 0; i < clients.size(); i++) {
    if (kill(clients[i], SIGTERM) < 0) {
      perror("TCPServer kill");
    }
  }

  join_clients();
}

// kill all clients
void Server::ClientHandler::kill_clients() {
  if (debug_mode) {
    fprintf(stderr, "Killing clients\n");
  }

  for (unsigned int i = 0; i < clients.size(); i++) {
    if (kill(clients[i], SIGKILL) < 0) {
      perror("TCPServer kill");
    }
  }

  join_clients();
}

Server::ClientHandler::~ClientHandler() { kill_clients(); }

void Server::ClientHandler::accept(int server_sock_fd) {
  reap_clients();
  if (debug_mode) {
    fprintf(stderr, "mode: %d, current_handler: %d\n", mode, current_handler);
  }
  ClientHandlerFunction handler;
  switch (mode) {
    case RoundRobin:
      handler = handlers[current_handler];
      current_handler = (current_handler + 1) % handlers.size();
      break;
    case Random:
      handler = handlers[rand() % handlers.size()];
      break;
    default:
      fprintf(stderr, "Invalid mode\n");
      throw std::runtime_error("Invalid mode");
  }

  struct sockaddr_in6 client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_sock_fd = ::accept(server_sock_fd, (struct sockaddr*)&client_addr,
                                &client_addr_len);
  if (client_sock_fd < 0) {
    perror("TCPClientHandler accept");
    return;
  }

  if (clients.size() >= max_clients) {
    if (debug_mode) {
      fprintf(stderr, "Max clients reached... dropping connection\n");
    }
    close(client_sock_fd);
    return;
  }
  if (!use_thread) {
    auto pid = fork();
    if (pid < 0) {
      perror("TCPClientHandler fork");
      close(client_sock_fd);
      return;
    }
    if (pid == 0) {
      // child process
      if (debug_mode) {
        fprintf(stderr, "Got new connection... creating TCPClient\n");
      }
      close(server_sock_fd);

      Client* client = new Client(client_sock_fd, client_addr);

      if (debug_mode) {
        fprintf(stderr, "Handling connection from %s\n", client->peer_ip());
      }

      if (debug_mode) {
        fprintf(stderr, "Calling handler\n");
      }

      handler(client, extra_data);
      delete client;
      exit(EXIT_SUCCESS);
    }
    close(client_sock_fd);
    if (debug_mode) {
      fprintf(stderr, "Adding child pid: %d\n", pid);
    }
    clients.push_back(pid);
  } else {
    std::thread handler_thread([handler, client_sock_fd, client_addr, this]() {
      if (debug_mode) {
        fprintf(stderr, "Got new connection... creating TCPClient\n");
      }

      Client* client = new Client(client_sock_fd, client_addr);

      if (debug_mode) {
        fprintf(stderr, "Handling connection from %s\n", client->peer_ip());
      }

      if (debug_mode) {
        fprintf(stderr, "Calling handler\n");
      }

      handler(client, extra_data);

      delete client;
    });
    handler_thread.detach();
  }
}

}  // namespace tcp