#include "udp_client.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <chrono>

UDPClient::UDPClient(sockaddr_in6 client_addr) {
  version = IPv6;
  peer_addr = client_addr;
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

  uint32_t handshake = HANDSHAKE_PHASE2;
  if (::send(sockfd, &handshake, sizeof(handshake), 0) < 0) {
    perror("UDPClient send");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "Sent handshake back\n");
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

  uint32_t handshake = HANDSHAKE_PHASE1;
  if (sendto(sockfd, &handshake, sizeof(handshake), 0,
             (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
    perror("UDPClient send");
    exit(EXIT_FAILURE);
  }

  socklen_t addr_len = sizeof(peer_addr);
  if (recvfrom(sockfd, &handshake, sizeof(handshake), 0,
               (struct sockaddr *)&peer_addr, &addr_len) < 0) {
    perror("UDPClient recvfrom");
    exit(EXIT_FAILURE);
  }

  if (handshake != HANDSHAKE_PHASE2) {
    fprintf(stderr, "UDPClient handshake failed\n");
    exit(EXIT_FAILURE);
  }

  // connect to the server's ephemeral port
  if (connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
    perror("UDPClient connect");
    exit(EXIT_FAILURE);
  }
}

UDPClient::~UDPClient() {
  if (close(sockfd) < 0) {
    perror("UDPClient close");
  }
}

size_t UDPClient::writen(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_written = ::write(sockfd, (char *)msgbuf + n, len - n);
    if (n_written < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("UDPClient write");
      exit(EXIT_FAILURE);
    }
    n += n_written;
  }
  if (n < len) {
    fprintf(stderr, "UDPClient writen: short write\n");
    exit(EXIT_FAILURE);
  }
  return n;
}

size_t UDPClient::readn(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_read = ::read(sockfd, (char *)msgbuf + n, len - n);
    if (n_read < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("UDPClient read");
      exit(EXIT_FAILURE);
    } else if (n_read == 0) {
      break;
    }
    n += n_read;
  }
  if (n < len) {
    fprintf(stderr, "UDPClient readn: short read\n");
    exit(EXIT_FAILURE);
  }
  return n;
}

size_t UDPClient::readline(void *msgbuf, size_t maxlen) {
  size_t n_read = 0;
  while (n_read < maxlen - 1) {
    ssize_t n = ::read(sockfd, (char *)msgbuf + n_read, 1);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("UDPClient read");
      exit(EXIT_FAILURE);
    } else if (n == 0) {
      break;
    }
    n_read += n;
    if (((char *)msgbuf)[n_read - 1] == '\n') {
      break;
    }
  }
  if (n_read == 0) {
    return 0;
  }
  if (((char *)msgbuf)[n_read - 1] != '\n') {
    fprintf(stderr, "UDPClient readline: no newline recieved\n");
    exit(EXIT_FAILURE);
  }
  ((char *)msgbuf)[n_read] = '\0';
  return n_read;
}

ssize_t UDPClient::write(void *msgbuf, size_t maxlen) {
  ssize_t n_written = ::write(sockfd, msgbuf, maxlen);
  return n_written;
}

ssize_t UDPClient::read(void *msgbuf, size_t maxlen) {
  ssize_t n_read = ::read(sockfd, msgbuf, maxlen);
  return n_read;
}