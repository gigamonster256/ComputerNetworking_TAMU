#include "tcp_client.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

TCPClient::TCPClient(int sockfd, sockaddr_in6 client_addr) : sockfd(sockfd) {
  if (inet_ntop(AF_INET6, &client_addr.sin6_addr, peer_ip_addr,
                sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
  }
}

TCPClient::TCPClient(const char *server, int port_no) {
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

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("TCPClient socket");
    exit(EXIT_FAILURE);
  }

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
}

TCPClient::~TCPClient() {
  if (close(sockfd) < 0) {
    perror("TCPClient close");
  }
}

size_t TCPClient::writen(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_written = ::write(sockfd, (char *)msgbuf + n, len - n);
    if (n_written < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("TCPClient write");
      exit(EXIT_FAILURE);
    }
    n += n_written;
  }
  if (n < len) {
    fprintf(stderr, "TCPClient writen: short write\n");
    exit(EXIT_FAILURE);
  }
  return n;
}

size_t TCPClient::readn(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_read = ::read(sockfd, (char *)msgbuf + n, len - n);
    if (n_read < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("TCPClient read");
      exit(EXIT_FAILURE);
    } else if (n_read == 0) {
      break;
    }
    n += n_read;
  }
  if (n < len) {
    fprintf(stderr, "TCPClient readn: short read\n");
    exit(EXIT_FAILURE);
  }
  return n;
}

size_t TCPClient::readline(void *msgbuf, size_t maxlen) {
  size_t n_read = 0;
  while (n_read < maxlen - 1) {
    ssize_t n = ::read(sockfd, (char *)msgbuf + n_read, 1);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("TCPClient read");
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
    fprintf(stderr, "TCPClient readline: no newline recieved\n");
    exit(EXIT_FAILURE);
  }
  ((char *)msgbuf)[n_read] = '\0';
  return n_read;
}

ssize_t TCPClient::write(void *msgbuf, size_t maxlen) {
  ssize_t n_written = ::write(sockfd, msgbuf, maxlen);
  return n_written;
}

ssize_t TCPClient::read(void *msgbuf, size_t maxlen) {
  ssize_t n_read = ::read(sockfd, msgbuf, maxlen);
  return n_read;
}