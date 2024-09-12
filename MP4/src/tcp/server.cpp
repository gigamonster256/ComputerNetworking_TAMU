#include "server.hpp"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>

namespace tcp {

TCPServer::~TCPServer() { stop(true); }

TCPServer& TCPServer::set_port(unsigned int port_no) {
  assert(server_pid < 0);
  assert(port_no > 0);
  this->port_no = port_no;
  return *this;
}

TCPServer& TCPServer::set_timeout(unsigned int timeout) {
  assert(server_pid < 0);
  this->timeout = timeout;
  return *this;
}

TCPServer& TCPServer::set_max_timeouts(unsigned int max_timeouts) {
  assert(server_pid < 0);
  this->max_timeouts = max_timeouts;
  return *this;
}

TCPServer& TCPServer::set_backlog(unsigned int backlog) {
  assert(server_pid < 0);
  assert(backlog > 0);
  this->backlog = backlog;
  return *this;
}

TCPServer& TCPServer::debug(bool mode) {
  assert(server_pid < 0);
  this->debug_mode = mode;
  client_handler.debug(mode);
  return *this;
}

TCPServer& TCPServer::set_timeout_handler(TimeoutFunction handler) {
  assert(server_pid < 0);
  this->timeout_handler = handler;
  return *this;
}

TCPServer& TCPServer::add_handler(ClientHandlerFunction handler) {
  assert(server_pid < 0);
  client_handler.add_handler(handler);
  return *this;
}

TCPServer& TCPServer::set_handler_mode(TCPClientHandler::handle_mode mode) {
  assert(server_pid < 0);
  client_handler.set_mode(mode);
  return *this;
}

TCPServer& TCPServer::set_max_clients(unsigned int max_clients) {
  assert(server_pid < 0);
  assert(max_clients > 0);
  client_handler.max_clients = max_clients;
  return *this;
}

TCPServer& TCPServer::add_handler_extra_data(void* data) {
  assert(server_pid < 0);
  client_handler.set_extra_data(data);
  return *this;
}

pid_t TCPServer::start() {
  // verify configuration
  assert(server_pid < 0);
  assert(port_no > 0);
  assert(backlog > 0);
  assert(client_handler.max_clients > 0);
  assert(client_handler.handlers.size() > 0);

  server_pid = fork();

  if (server_pid < 0) {
    perror("TCPServer fork");
    return server_pid;
  }

  if (server_pid == 0) {
    run_server();
    exit(EXIT_SUCCESS);
  }

  return server_pid;
}

void TCPServer::exec() {
  auto pid = start();

  if (pid < 0) {
    perror("TCPServer exec");
    exit(EXIT_FAILURE);
  }

  if (waitpid(pid, nullptr, 0) < 0) {
    perror("TCPServer waitpid");
  }
  exit(EXIT_SUCCESS);
}

void TCPServer::run_server() {
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
  server_addr.sin6_addr = in6addr_any;

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

void TCPServer::stop(bool force) {
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
void TCPServer::TCPClientHandler::reap_clients() {
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
void TCPServer::TCPClientHandler::join_clients() {
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
void TCPServer::TCPClientHandler::terminate_clients() {
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
void TCPServer::TCPClientHandler::kill_clients() {
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

TCPServer::TCPClientHandler::~TCPClientHandler() { kill_clients(); }

void TCPServer::TCPClientHandler::accept(int server_sock_fd) {
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

    TCPClient* client = new TCPClient(client_sock_fd, client_addr);

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
}

}  // namespace tcp