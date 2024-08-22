#include "tcp_server.hpp"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <utility>

bool TCPServer::should_exit = false;
bool TCPServer::debug_mode = false;

TCPServer::TCPServer()
    : port_no(-1),
      version(IPv6),
      server_sock_fd(-1),
      timeout(1),
      max_timeouts(0),
      timeout_handler(NULL),
      backlog(5),
      num_children(0),
      max_children(1),
      server_pid(-1),
      sock_opened(false) {
  child_pids = new pid_t[max_children];
}

TCPServer::~TCPServer() {
  delete[] child_pids;
  // socket closed in join_children
}

TCPServer& TCPServer::set_port(int port_no) {
  modification_after_start_error("set_port");
  this->port_no = port_no;
  return *this;
}

TCPServer& TCPServer::add_handler(ClientHandlerFunction func) {
  modification_after_start_error("add_handler");
  client_handler.add_handler(func);
  return *this;
}

TCPServer& TCPServer::set_handler_mode(TCPClientHandler::handle_mode mode) {
  modification_after_start_error("set_handler_mode");
  client_handler.set_mode(mode);
  return *this;
}

TCPServer& TCPServer::set_timeout(int seconds) {
  if (seconds <= 0) {
    fprintf(stderr, "TCPServer Error: timeout must be > 0\n");
    return *this;
  }
  modification_after_start_error("set_timeout");
  timeout = seconds;
  return *this;
}

TCPServer& TCPServer::set_max_timeouts(int seconds) {
  modification_after_start_error("set_max_timeouts");
  max_timeouts = seconds;
  return *this;
}

void TCPServer::modification_after_start_error(const char* method) {
  if (started()) {
    fprintf(stderr, "TCPServer Error: %s called after server started\n",
            method);
  }
}

TCPServer& TCPServer::set_timeout_handler(TimeoutFunction handler) {
  modification_after_start_error("set_timeout_handler");
  timeout_handler = handler;
  return *this;
}

TCPServer& TCPServer::set_backlog(int size) {
  modification_after_start_error("set_backlog");
  backlog = size;
  return *this;
}

TCPServer& TCPServer::debug(bool mode) {
  debug_mode = mode;
  return *this;
}

pid_t TCPServer::start() {
  if (started()) {
    fprintf(stderr,
            "TCPServer Warning: start called when server already started\n");
    return server_pid;
  }

  if (port_no == -1) {
    fprintf(stderr, "TCPServer Error: port number not set\n");
    exit(EXIT_FAILURE);
  }
  if (version == IPv4) {
    fprintf(stderr, "TCPServer Error: IPv4 not supported\n");
    exit(EXIT_FAILURE);
  }

  if (client_handler.num_handlers == 0) {
    fprintf(stderr, "TCPServer Error: no handlers added\n");
    exit(EXIT_FAILURE);
  }

  if (debug_mode) {
    fprintf(stderr, "Starting TCPServer\n");
  }

  auto pid = fork();

  if (pid < 0) {
    perror("TCPServer fork");
    return pid;
  }

  if (pid == 0) {
    start_server();
    exit(EXIT_SUCCESS);
  }

  server_pid = pid;

  return server_pid;
}

void TCPServer::exec() {
  auto pid = start();

  if (debug_mode) {
    fprintf(stderr, "Waiting for server process to exit\n");
  }

  int status;
  if (waitpid(pid, &status, 0) < 0) {
    perror("TCPServer waitpid");
  }
}

void TCPServer::start_server() {

  if (debug_mode) {
    fprintf(stderr, "Starting server process\n");
  }

  server_sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (server_sock_fd < 0) {
    perror("TCPServer socket");
    exit(EXIT_FAILURE);
  }

  sock_opened = true;

  struct sockaddr_in6 server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port_no);
  server_addr.sin6_addr = in6addr_any;

  if (bind(server_sock_fd, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("TCPServer bind");
    exit(EXIT_FAILURE);
  }

  if (listen(server_sock_fd, backlog) < 0) {
    perror("TCPServer listen");
    exit(EXIT_FAILURE);
  }

  if (debug_mode) {
    fprintf(stderr, "TCPServer started on port %d\n", port_no);
  }

  while (true && !should_exit) {
    reap_children();

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
      }

      if (ret == 0) {
        if (debug_mode) {
          fprintf(stderr, "TCPServer timeout\n");
        }

        timeout_counter++;
        if (timeout_handler != NULL) {
          if (debug_mode) {
            fprintf(stderr, "Calling timeout handler\n");
          }
          timeout_handler();
          continue;
        }
        if (max_timeouts > 0 && timeout_counter >= max_timeouts) {
          if (debug_mode) {
            fprintf(stderr, "Max timeouts reached\n");
          }
          break;
        }
        continue;
      }
    }

    timeout_counter = 0;

    struct sockaddr_in6 client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock_fd = accept(server_sock_fd, (struct sockaddr*)&client_addr,
                                &client_addr_len);
    if (client_sock_fd < 0) {
      perror("TCPServer accept");
      exit(EXIT_FAILURE);
    }

    if (debug_mode) {
      fprintf(stderr, "Got new connection... creating TCPClient\n");
    }

    TCPClient* client = new TCPClient(client_sock_fd, client_addr);

    if (debug_mode) {
      fprintf(stderr, "Handling connection from %s\n", client->ip());
    }

    auto pid = client_handler.handle(client);
    if (pid < 0) {
      perror("TCPServer client_handler");
      delete client;
      continue;
    }

    add_child(pid);
  }

  if (debug_mode) {
    fprintf(stderr, "Stopping server thread\n");
  }
  join_children();
}

void TCPServer::add_child(pid_t pid) {
  if (num_children == max_children) {
    max_children *= 2;
    pid_t* new_child_pids = new pid_t[max_children];
    memcpy(new_child_pids, child_pids, num_children * sizeof(pid_t));
    delete[] child_pids;
    child_pids = new_child_pids;
  }
  child_pids[num_children++] = pid;
}

void TCPServer::reap_children() {
  if (debug_mode) {
    fprintf(stderr, "Checking for exited child processes\n");
  }
  for (int i = 0; i < num_children; i++) {
    int status;
    if (waitpid(child_pids[i], &status, WNOHANG) > 0) {
      if (debug_mode) {
        fprintf(stderr, "Child process %d exited\n", child_pids[i]);
      }
      for (int j = i; j < num_children - 1; j++) {
        child_pids[j] = child_pids[j + 1];
      }
      num_children--;
      i--;
    }
  }
}

void TCPServer::join_children() {
  // stop accepting new connections
  if (sock_opened && (server_sock_fd) < 0) {
    perror("TCPServer close");
  }

  if (debug_mode) {
    fprintf(stderr, "Closed server socket\n");
  }

  if (debug_mode && num_children > 0) {
    fprintf(stderr, "Waiting for all child processes to exit\n");
    fprintf(stderr, "num_children: %d\n", num_children);
  }
  for (int i = 0; i < num_children; i++) {
    int status;
    if (waitpid(child_pids[i], &status, 0) < 0) {
      perror("TCPServer waitpid");
    }
  }
}

void TCPServer::stop() {
  if (!started()) {
    fprintf(stderr, "TCPServer Warning: stop called when server not started\n");
    return;
  }

  if (debug_mode) {
    fprintf(stderr, "Stopping TCPServer\n");
  }

  if (kill(server_pid, SIGTERM) < 0) {
    perror("TCPServer kill");
  }
  waitpid(server_pid, NULL, 0);
}

TCPServer::TCPClientHandler::TCPClientHandler()
    : max_handlers(1),
      num_handlers(0),
      handlers(NULL),
      current_handler(0),
      mode(RoundRobin) {
  handlers = new ClientHandlerFunction[max_handlers];
}

TCPServer::TCPClientHandler::~TCPClientHandler() { delete[] handlers; }

void TCPServer::TCPClientHandler::add_handler(ClientHandlerFunction handler) {
  if (TCPServer::debug_mode) {
    fprintf(stderr, "Adding handler\n");
  }
  if (num_handlers == max_handlers) {
    if (TCPServer::debug_mode) {
      fprintf(stderr, "Resizing handlers array\n");
    }
    max_handlers *= 2;
    ClientHandlerFunction* new_handlers =
        new ClientHandlerFunction[max_handlers];
    memcpy(new_handlers, handlers,
           num_handlers * sizeof(ClientHandlerFunction));
    delete[] handlers;
    handlers = new_handlers;
  }
  handlers[num_handlers++] = handler;
  if (TCPServer::debug_mode) {
    fprintf(stderr, "Handler added... %d installed\n", num_handlers);
  }
}

void TCPServer::TCPClientHandler::set_mode(handle_mode mode) {
  this->mode = mode;
}

pid_t TCPServer::TCPClientHandler::handle(TCPClient* client) {
  if (TCPServer::debug_mode) {
    fprintf(stderr, "mode: %d, current_handler: %d\n", mode, current_handler);
  }
  ClientHandlerFunction handler;
  switch (mode) {
    case RoundRobin:
      handler = handlers[current_handler];
      current_handler = (current_handler + 1) % num_handlers;
      break;
    case Random:
      handler = handlers[rand() % num_handlers];
      break;
  }
  auto pid = fork();
  if (pid < 0) {
    perror("TCPClientHandler fork");
    return pid;
  }
  if (pid == 0) {
    if (TCPServer::debug_mode) {
      fprintf(stderr, "Calling handler\n");
    }
    handler(client);
    exit(EXIT_SUCCESS);
  }
  return pid;
}