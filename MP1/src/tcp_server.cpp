#include "tcp_server.hpp"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <utility>

TCPServer::~TCPServer() {
  if (server_sock_fd >= 0) {
    stop(true);
  }
}

TCPServer& TCPServer::set_port(unsigned int port_no) {
  assert(server_pid < 0);
  assert(port_no > 0);
  this->port_no = port_no;
  return *this;
}

TCPServer& TCPServer::set_timeout(unsigned int timeout) {
  assert(server_pid < 0);
  assert(timeout >= 0);
  this->timeout = timeout;
  return *this;
}

TCPServer& TCPServer::set_max_timeouts(unsigned int max_timeouts) {
  assert(server_pid < 0);
  assert(max_timeouts >= 0);
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

pid_t TCPServer::start() {
  // verify configuration
  assert(server_pid < 0);
  assert(port_no > 0);
  assert(timeout >= 0);
  assert(max_timeouts >= 0);
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
    client_handler.reap_children();

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

    struct sockaddr_in6 client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock_fd = accept(server_sock_fd, (struct sockaddr*)&client_addr,
                                &client_addr_len);
    if (client_sock_fd < 0) {
      perror("TCPServer accept");
      break;
    }

    // pass client to handler
    client_handler.handle(client_sock_fd, client_addr);
  }

  if (debug_mode) {
    fprintf(stderr, "Stopping server thread\n");
  }

  client_handler.join_children();
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

// reap all children that have finished
void TCPServer::TCPClientHandler::reap_children() {
  if (debug_mode) {
    fprintf(stderr, "Reaping children\n");
  }

  std::vector<pid_t> finished_children;
  while (pid_t pid = waitpid(-1, nullptr, WNOHANG) > 0) {
    finished_children.push_back(pid);
  }

  if (debug_mode) {
    fprintf(stderr, "Reaped %lu children\n", finished_children.size());
  }

  children.erase(std::remove_if(children.begin(), children.end(),
                                [&finished_children](pid_t pid) {
                                  return std::find(finished_children.begin(),
                                                   finished_children.end(),
                                                   pid) !=
                                         finished_children.end();
                                }),
                 children.end());
}

// wait for all children to finish
void TCPServer::TCPClientHandler::join_children() {
  if (debug_mode) {
    fprintf(stderr, "Joining children\n");
  }

  for (unsigned int i = 0; i < children.size(); i++) {
    if (waitpid(children[i], nullptr, 0) < 0) {
      perror("TCPServer waitpid");
    }
  }
  children.clear();
}

// tell all children to terminate
void TCPServer::TCPClientHandler::terminate_children() {
  if (debug_mode) {
    fprintf(stderr, "Terminating children\n");
  }

  for (unsigned int i = 0; i < children.size(); i++) {
    if (kill(children[i], SIGTERM) < 0) {
      perror("TCPServer kill");
    }
  }

  join_children();
}

// kill all children
void TCPServer::TCPClientHandler::kill_children() {
  if (debug_mode) {
    fprintf(stderr, "Killing children\n");
  }

  for (unsigned int i = 0; i < children.size(); i++) {
    if (kill(children[i], SIGKILL) < 0) {
      perror("TCPServer kill");
    }
  }

  join_children();
}

TCPServer::TCPClientHandler::~TCPClientHandler() { kill_children(); }

void TCPServer::TCPClientHandler::handle(int client_sock_fd,
                                         struct sockaddr_in6 client_addr) {
  reap_children();
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

  if (children.size() >= max_clients) {
    if (debug_mode) {
      fprintf(stderr, "Max clients reached... dropping connection\n");
    }
    close(client_sock_fd);
    return;
  }

  auto pid = fork();
  if (pid < 0) {
    perror("TCPClientHandler fork");
    return;
  }
  if (pid == 0) {
    if (debug_mode) {
      fprintf(stderr, "Got new connection... creating TCPClient\n");
    }

    TCPClient* client = new TCPClient(client_sock_fd, client_addr);

    if (debug_mode) {
      fprintf(stderr, "Handling connection from %s\n", client->peer_ip());
    }

    if (debug_mode) {
      fprintf(stderr, "Calling handler\n");
    }

    handler(client);
    delete client;
    exit(EXIT_SUCCESS);
  }
  if (debug_mode) {
    fprintf(stderr, "Adding child pid: %d\n", pid);
  }
  children.push_back(pid);
}