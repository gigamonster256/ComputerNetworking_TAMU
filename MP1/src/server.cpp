// tcp echo server
// support multiple clients

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "echo.hpp"

void usage(const char *progname) {
  std::cerr << "Usage: " << progname << " <port>" << std::endl;
  exit(1);
}

void echo_back(int client_fd, int client_id) {
  char buffer[MAX_MESSAGE];
  std::cerr << "Client " << client_id << " connected" << std::endl;
  while (true) {
    size_t len_recvd = recv(client_fd, buffer, MAX_MESSAGE, 0);
    if (len_recvd < 0) {
      perror("recv");
      return;
    }

    // client closed connection
    if (len_recvd == 0) {
      std::cerr << "Client " << client_id << " closed connection" << std::endl;
      close(client_fd);
      return;
    }

    // echo back to client
    size_t len_sent = send(client_fd, buffer, len_recvd, 0);
    if (len_sent < 0) {
      perror("send");
      return;
    }
    if (len_sent != len_recvd) {
      std::cerr << "send: sent " << len_sent << " bytes" << std::endl;
      std::cerr << "send: expected to send " << len_recvd << " bytes"
                << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage(argv[0]);
  }

  int port = atoi(argv[1]);

  int server_fd = socket(AF_INET6, SOCK_STREAM, 0);

  if (server_fd < 0) {
    perror("socket");
    exit(1);
  }

  // bind to port and listen for connections
  struct sockaddr_in6 server;
  memset(&server, 0, sizeof(server));
  server.sin6_family = AF_INET6;
  server.sin6_port = htons(port);
  server.sin6_addr = in6addr_any;
  if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  // accept connections and echo messages
  std::cout << "Listening on port " << port << std::endl;
  int client_id = 0;
  while (true) {
    struct sockaddr_in6 client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
      perror("accept");
      return 1;
    }

    char client_ip[INET6_ADDRSTRLEN];
    const char *dst = inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip,
                                sizeof(client_ip));
    if (dst == NULL) {
      perror("inet_ntop");
      return 1;
    }

    std::cerr << "Accepted connection from " << dst << ":"
              << ntohs(client_addr.sin6_port) << std::endl;

    std::thread t(echo_back, client_fd, client_id++);
    t.detach();
  }
}
