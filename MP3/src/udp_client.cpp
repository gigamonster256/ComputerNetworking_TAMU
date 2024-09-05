#include "udp_client.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <chrono>

UDPClient::UDPClient(const struct sockaddr_in6& client_addr) {
  version = IPv6;
  peer_addr = client_addr;
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

UDPClient::UDPClient(const char *server, int port_no) {
  fprintf(stderr, "Connecting to server %s\n", server);
  fprintf(stderr, "Port: %d\n", port_no);
  // figure out if we are using IPv4 or IPv6 based on the server address
  version = IPv4;
  // simple check for colons in the address
  for (auto p = server; *p; p++) {
    if (*p == ':') {
      version = IPv6;
      break;
    }
  }
  // convert to v6 if it is v4
  if (version == IPv4) {
    snprintf(peer_ip_addr, sizeof(peer_addr), "::ffff:%s", server);
    version = IPv6;
  } else {
    strcpy(peer_ip_addr, server);
  }

  sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("UDPClient socket");
    exit(EXIT_FAILURE);
  }

  memset(&peer_addr, 0, sizeof(peer_addr));
  peer_addr.sin6_family = AF_INET6;
  peer_addr.sin6_port = htons(port_no);
  if (inet_pton(AF_INET6, peer_ip_addr, &peer_addr.sin6_addr) < 0) {
    perror("UDPClient inet_pton");
    exit(EXIT_FAILURE);
  }
}

UDPClient::~UDPClient() {
  if (close(sockfd) < 0) {
    perror("UDPClient close");
  }
}

void UDPClient::connect_to_ephemeral_port(const struct sockaddr_in6& server_addr) {
  // connect to the server's ephemeral port
  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("UDPClient connect");
    exit(EXIT_FAILURE);
  }
  peer_addr = server_addr;
  if (inet_ntop(AF_INET6, &server_addr.sin6_addr, peer_ip_addr, sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
  }
  connected_to_ephemeral_port = true;
}

ssize_t UDPClient::write(void *msgbuf, size_t maxlen) {
  ssize_t n_written;
  if (!connected_to_ephemeral_port) {
    // send to the server's well known port
    n_written = sendto(sockfd, msgbuf, maxlen, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    if (n_written < 0) {
      perror("UDPClient sendto");
    }
  } else {
    n_written = ::write(sockfd, msgbuf, maxlen);
  }
  return n_written;
}

ssize_t UDPClient::read(void *msgbuf, size_t maxlen) {
  ssize_t n_read;
  if (!connected_to_ephemeral_port) {
    struct sockaddr_in6 server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    n_read = recvfrom(sockfd, msgbuf, maxlen, 0, (struct sockaddr*)&server_addr, &server_addr_len);
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