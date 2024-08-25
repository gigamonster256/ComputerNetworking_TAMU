// tcp echo server
// support multiple clients

#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "tcp_server.hpp"

void usage(const char *progname) {
  std::cerr << "Usage: " << progname << " <port>" << std::endl;
  exit(EXIT_FAILURE);
}

void echo_handler(TCPClient *client) {
  fprintf(stderr, "I am echo_handler with pid %d\n", getpid());
  fprintf(stderr, "Accepted connection from %s\n", client->ip());
  char buffer[1024];
  while (true) {
    int bytes_read = client->read(buffer, sizeof(buffer));
    if (bytes_read < 0) {
      perror("read");
      break;
    }
    if (bytes_read == 0) {
      fprintf(stderr, "Connection closed by client\n");
      break;
    }

    int bytes_written = client->writen(buffer, bytes_read);
    if (bytes_written < 0) {
      perror("write");
      break;
    }
  }
}

void ohce_handler(TCPClient *client) {
  fprintf(stderr, "I am ohce_handler with pid %d\n", getpid());
  fprintf(stderr, "Accepted connection from %s\n", client->ip());
  char buffer[1024];
  while (true) {
    // read an entire line (nescessary to know when string ends)
    int bytes_read = client->readline(buffer, sizeof(buffer));
    if (bytes_read < 0) {
      perror("read");
      break;
    }
    if (bytes_read == 0) {
      fprintf(stderr, "Connection closed by client\n");
      break;
    }

    // reverse the string (echo vs ohce)
    for (int i = 0; i < bytes_read / 2; i++) {
      char tmp = buffer[i];
      buffer[i] = buffer[bytes_read - i - 2];
      buffer[bytes_read - i - 2] = tmp;
    }

    int bytes_written = client->writen(buffer, bytes_read);
    if (bytes_written < 0) {
      perror("write");
      break;
    }
  }
}

void timeout_handler() {
  fprintf(stderr, "No client in the last select interval\n");
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage(argv[0]);
  }

  int port = atoi(argv[1]);

  TCPServer server;
  server.set_port(port)
      .add_handler(echo_handler)
      // .set_timeout(100)
      .set_timeout_handler(timeout_handler)
      // .debug(true)
      .exec();
}
