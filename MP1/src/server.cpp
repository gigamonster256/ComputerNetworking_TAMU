// tcp echo server
// support multiple clients

#include <stdio.h>
#include <stdlib.h>

#include "tcp_server.hpp"

void usage(const char *progname) {
  fprintf(stderr, "Usage: %s <port>\n", progname);
  exit(EXIT_FAILURE);
}

void echo_handler(TCPClient *client, void *) {
  char buffer[1024];
  while (true) {
    int bytes_read = client->read(buffer, sizeof(buffer));
    if (bytes_read < 0) {
      perror("read");
      break;
    }

    // EOF
    if (bytes_read == 0) {
      break;
    }

    int bytes_written = client->writen(buffer, bytes_read);
    if (bytes_written < 0 || bytes_written != bytes_read) {
      perror("write");
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage(argv[0]);
  }

  int port = atoi(argv[1]);

  TCPServer server;
  server.set_port(port).add_handler(echo_handler).exec();
}
