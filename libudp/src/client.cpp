#include "client.hpp"

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace udp {

Client::Client(const struct sockaddr_in6 &client_addr) {
  connected_to_ephemeral_port = true;
  sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("UDPClient socket");
    exit(EXIT_FAILURE);
  }
  if (inet_ntop(AF_INET6, &client_addr.sin6_addr, peer_ip_addr,
                sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
  }

  fprintf(stderr, "Connected to client %s\n", peer_ip_addr);
  fprintf(stderr, "Port: %d\n", ntohs(client_addr.sin6_port));

  if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) <
      0) {
    perror("UDPClient connect");
    exit(EXIT_FAILURE);
  }
}

Client::Client(const char *server, int port_no) {
  fprintf(stderr, "Connecting to server %s\n", server);
  fprintf(stderr, "Port: %d\n", port_no);
  // figure out if we are using IPv4 or IPv6 based on the server address
  // simple check for colons in the address
  bool is_ipv6 = false;
  for (auto p = server; *p; p++) {
    if (*p == ':') {
      is_ipv6 = true;
      break;
    }
  }

  // convert to v6 if it is v4
  if (!is_ipv6) {
    snprintf(peer_ip_addr, INET6_ADDRSTRLEN, "::ffff:%s", server);
  } else {
    strncpy(peer_ip_addr, server, INET6_ADDRSTRLEN);
  }

  sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("UDPClient socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port_no);
  if (inet_pton(AF_INET6, peer_ip_addr, &server_addr.sin6_addr) < 0) {
    perror("UDPClient inet_pton");
    exit(EXIT_FAILURE);
  }
}

Client::~Client() {
  if (close(sockfd) < 0) {
    perror("UDPClient close");
  }
}

void Client::connect_to_ephemeral_port(const struct sockaddr_in6 &server_addr) {
  // connect to the server's ephemeral port
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("UDPClient connect");
    exit(EXIT_FAILURE);
  }
  if (inet_ntop(AF_INET6, &server_addr.sin6_addr, peer_ip_addr,
                sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
  }
  connected_to_ephemeral_port = true;
}

ssize_t Client::write(void *msgbuf, size_t maxlen) {
  ssize_t n_written;
  if (!connected_to_ephemeral_port) {
    // send to the server's well known port
    n_written = sendto(sockfd, msgbuf, maxlen, 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n_written < 0) {
      perror("UDPClient sendto");
    }
  } else {
    n_written = ::write(sockfd, msgbuf, maxlen);
  }
  return n_written;
}

ssize_t Client::read(void *msgbuf, size_t maxlen) {
  ssize_t n_read;
  if (!connected_to_ephemeral_port) {
    // read from the server's ephemeral port
    struct sockaddr_in6 server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    n_read = recvfrom(sockfd, msgbuf, maxlen, 0,
                      (struct sockaddr *)&server_addr, &server_addr_len);
    if (n_read < 0) {
      perror("UDPClient recvfrom");
    }
    // start listening only to the server's ephemeral port
    connect_to_ephemeral_port(server_addr);
  } else {
    n_read = ::read(sockfd, msgbuf, maxlen);
  }
  return n_read;
}

}  // namespace udp