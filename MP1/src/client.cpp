// tcp echo client

#include <iostream>

#include "tcp_client.hpp"

void usage(const char *progname) {
  std::cerr << "Usage: " << progname << " <server> <port>" << std::endl;
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
  while(char* s = fgets(buf, sizeof(buf), stdin)) {
    client.writen(buf, strlen(buf));
    client.readline(buf, sizeof(buf));
    fputs(buf, stdout);
  }
  return 0;
}