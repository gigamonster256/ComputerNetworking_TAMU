// tcp echo client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp/client.hpp"

using namespace tcp;

void usage(const char *progname) {
  fprintf(stderr, "Usage: %s <server> <port>\n", progname);
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    usage(argv[0]);
  }

  const char *server = argv[1];
  int port = atoi(argv[2]);

  Client client(server, port);
  char buf[512];

  // read from stdin until EOF
  while (fgets(buf, sizeof(buf), stdin)) {
    // write stdin to server until newline is encountered
    size_t len = strlen(buf);
    client.writen(buf, len);
    if (buf[len - 1] != '\n') {
      continue;
    }

    // read until newline is encountered
    do {
      len = client.readline(buf, sizeof(buf));
      if (len == 0) {
        // server closed connection :(
        exit(EXIT_FAILURE);
      }
      fputs(buf, stdout);
    } while (buf[len - 1] != '\n');
  }
  return 0;
}