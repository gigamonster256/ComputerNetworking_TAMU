// tcp echo client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_client.hpp"

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

  TCPClient client(server, port);
  char buf[1024];
  while (fgets(buf, sizeof(buf), stdin)) {
    client.writen(buf, strlen(buf));
    client.readline(buf, sizeof(buf));
    fputs(buf, stdout);
  }
  return 0;
}