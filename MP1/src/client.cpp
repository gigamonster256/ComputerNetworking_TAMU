// tcp echo client

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "echo.hpp"

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

  // create a socket
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd < 0) {
    perror("socket");
    exit(1);
  }

  // connect to the server
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  int r = inet_pton(AF_INET, server, &server_addr.sin_addr);
  if (r < 0) {
    perror("inet_pton");
    exit(1);
  } else if (r == 0) {
    std::cerr << "Invalid address: " << server <<std::endl;
    exit(1);
  }
  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect");
    exit(1);
  }

  // read from stdin and send to server
  char buffer[MAX_MESSAGE];
  while (fgets(buffer, MAX_MESSAGE, stdin)) {
    send(client_fd, buffer, strlen(buffer), 0);
    int len = recv(client_fd, buffer, MAX_MESSAGE, 0);
    if (len < 0) {
      perror("recv");
      exit(1);
    }
    buffer[len] = '\0';
    fputs(buffer, stdout);
  }

  close(client_fd);

  return 0;
}