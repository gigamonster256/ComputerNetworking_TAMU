#include "udp/server.hpp"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdexcept>
#include <algorithm>

namespace udp {

Server::~Server() { stop(true); }

Server& Server::set_port(unsigned int port_no) {
  assert(server_pid < 0);
  assert(port_no > 0);
  this->port_no = port_no;
  return *this;
}

Server& Server::set_timeout(unsigned int timeout) {
  assert(server_pid < 0);
  this->timeout = timeout;
  return *this;
}

Server& Server::set_max_timeouts(unsigned int max_timeouts) {
  assert(server_pid < 0);
  this->max_timeouts = max_timeouts;
  return *this;
}

Server& Server::debug(bool mode) {
  assert(server_pid < 0);
  this->debug_mode = mode;
  client_handler.debug(mode);
  return *this;
}

Server& Server::set_timeout_handler(TimeoutFunction handler) {
  assert(server_pid < 0);
  this->timeout_handler = handler;
  return *this;
}

Server& Server::add_handler(ClientHandlerFunction handler) {
  assert(server_pid < 0);
  client_handler.add_handler(handler);
  return *this;
}

Server& Server::set_handler_mode(ClientHandler::handle_mode mode) {
  assert(server_pid < 0);
  client_handler.set_mode(mode);
  return *this;
}

Server& Server::set_max_clients(unsigned int max_clients) {
  assert(server_pid < 0);
  assert(max_clients > 0);
  client_handler.max_clients = max_clients;
  return *this;
}

Server& Server::add_handler_extra_data(void* data) {
  assert(server_pid < 0);
  client_handler.set_extra_data(data);
  return *this;
}

pid_t Server::start() {
  // verify configuration
  assert(server_pid < 0);
  assert(port_no > 0);
  assert(client_handler.max_clients > 0);
  assert(client_handler.handlers.size() > 0);

  server_pid = fork();

  if (server_pid < 0) {
    perror("UDPServer fork");
    return server_pid;
  }

  if (server_pid == 0) {
    run_server();
    exit(EXIT_SUCCESS);
  }

  return server_pid;
}

void Server::exec() {
  auto pid = start();

  if (pid < 0) {
    perror("UDPServer exec");
    exit(EXIT_FAILURE);
  }

  if (waitpid(pid, nullptr, 0) < 0) {
    perror("UDPServer waitpid");
  }
  exit(EXIT_SUCCESS);
}

void Server::run_server() {
  if (debug_mode) {
    fprintf(stderr, "Starting server process pid: %d\n", getpid());
  }

  server_sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (server_sock_fd < 0) {
    perror("UDPServer socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in6 server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port_no);
  server_addr.sin6_addr = in6addr_any;

  if (bind(server_sock_fd, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("UDPServer bind");
    close(server_sock_fd);
    exit(EXIT_FAILURE);
  }

  if (debug_mode) {
    fprintf(stderr, "UDPServer started on port %d\n", port_no);
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
        perror("UDPServer select");
        if (errno == EINTR) {
          continue;
        }
        // stop server on error
        break;
      }

      if (ret == 0) {
        if (debug_mode) {
          fprintf(stderr, "UDPServer timeout\n");
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
      perror("UDPServer kill");
    }
  } else {
    if (kill(server_pid, SIGTERM) < 0) {
      perror("UDPServer kill");
    }
  }

  if (waitpid(server_pid, nullptr, 0) < 0) {
    perror("UDPServer waitpid");
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
      perror("UDPServer waitpid");
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
      perror("UDPServer waitpid");
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
      perror("UDPServer kill");
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
      perror("UDPServer kill");
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
  char* first_packet = new char[initial_packet_buffer_size];
  ssize_t len =
      recvfrom(server_sock_fd, first_packet, initial_packet_buffer_size, 0,
               (struct sockaddr*)&client_addr, &client_addr_len);
  if (len < 0) {
    perror("UDPServer recvfrom");
    return;
  }

  if (clients.size() >= max_clients) {
    if (debug_mode) {
      fprintf(stderr, "Max clients reached... dropping connection\n");
    }
    return;
  }

  auto pid = fork();
  if (pid < 0) {
    perror("UDPClientHandler fork");
    return;
  }
  if (pid == 0) {
    // child process
    if (debug_mode) {
      fprintf(stderr, "Got new connection... creating UDPClient\n");
    }
    close(server_sock_fd);

    Client* client = new Client(client_addr);

    if (debug_mode) {
      fprintf(stderr, "Handling connection from %s\n", client->peer_ip());
    }

    if (debug_mode) {
      fprintf(stderr, "Calling handler\n");
    }

    handler(client, first_packet, len, extra_data);
    delete client;
    delete[] first_packet;
    exit(EXIT_SUCCESS);
  }
  if (debug_mode) {
    fprintf(stderr, "Adding child pid: %d\n", pid);
  }
  clients.push_back(pid);
}

}  // namespace udp