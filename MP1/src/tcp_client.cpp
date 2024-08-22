#include "tcp_client.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

TCPClient::TCPClient(int sockfd, sockaddr_in6 client_addr) : sockfd(sockfd) {
  if (inet_ntop(AF_INET6, &client_addr.sin6_addr, ip_addr, sizeof(ip_addr)) ==
      NULL) {
    perror("TCPClient inet_ntop");
    exit(EXIT_FAILURE);
  }
}

TCPClient::TCPClient(const char *server, int port_no) {
  // figure out if we are using IPv4 or IPv6 based on the server address
  version = IPv4;
  // simple check for colons in the address
  for (auto p = server; *p; p++) {
    if (*p == ':') {
      version = IPv6;
      break;
    }
  }

  sockfd = socket(version == IPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("TCPClient socket");
    exit(EXIT_FAILURE);
  }

  switch (version) {
    case IPv4: {
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port_no);
      if (inet_pton(AF_INET, server, &addr.sin_addr) < 0) {
        perror("TCPClient inet_pton");
        exit(EXIT_FAILURE);
      }

      if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("TCPClient connect");
        exit(EXIT_FAILURE);
      }
      break;
    }
    case IPv6: {
      struct sockaddr_in6 addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin6_family = AF_INET6;
      addr.sin6_port = htons(port_no);
      if (inet_pton(AF_INET6, server, &addr.sin6_addr) < 0) {
        perror("TCPClient inet_pton");
        exit(EXIT_FAILURE);
      }

      if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("TCPClient connect");
        exit(EXIT_FAILURE);
      }
      break;
    }
  }
}

TCPClient::~TCPClient() {
  if (close(sockfd) < 0) {
    perror("TCPClient close");
  }
}

ssize_t TCPClient::writen(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_written = write(sockfd, (char *)msgbuf + n, len - n);
    if (n_written < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("TCPClient write");
      return n_written;
    }
    n += n_written;
  }
  return n;
}

ssize_t TCPClient::readline(void *msgbuf, size_t maxlen) {
  size_t n_read = 0;
  while (n_read < maxlen - 1) {
    ssize_t n = ::read(sockfd, (char *)msgbuf + n_read, 1);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("TCPClient read");
      return n;
    } else if (n == 0) {
      break;
    }
    n_read += n;
    if (((char *)msgbuf)[n_read - 1] == '\n') {
      break;
    }
  }
  ((char *)msgbuf)[n_read] = '\0';
  return n_read;
}

ssize_t TCPClient::read(void *msgbuf, size_t len) {
  ssize_t n_read = ::read(sockfd, msgbuf, len);
  if (n_read < 0) {
    perror("TCPClient read");
    return n_read;
  }
  return n_read;
}