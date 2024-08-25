#include "tcp_client.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <chrono>

TCPClient::TCPClient(int sockfd, sockaddr_in6 client_addr) : sockfd(sockfd) {
  if (inet_ntop(AF_INET6, &client_addr.sin6_addr, peer_ip_addr,
                sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
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

ssize_t TCPClient::readn_timeout(void *msgbuf, size_t len, int timeout_sec) {
  ssize_t n = select(timeout_sec);
  if (n < 0) {
    perror("TCPClient select");
    exit(EXIT_FAILURE);
  } else if (n == 0) {
    errno = ETIMEDOUT;
    return -1;
  }
  return readn(msgbuf, len);
}

ssize_t TCPClient::readline_timeout(void *msgbuf, size_t maxlen,
                                    int timeout_sec) {
  ssize_t n = select(timeout_sec);
  if (n < 0) {
    perror("TCPClient select");
    exit(EXIT_FAILURE);
  } else if (n == 0) {
    errno = ETIMEDOUT;
    return -1;
  }
  return readline(msgbuf, maxlen);
}

ssize_t TCPClient::read_timeout(void *msgbuf, size_t maxlen, int timeout_sec) {
  ssize_t n = select(timeout_sec);
  if (n < 0) {
    perror("TCPClient select");
    exit(EXIT_FAILURE);
  } else if (n == 0) {
    errno = ETIMEDOUT;
    return -1;
  }
  return read(msgbuf, maxlen);
}

int TCPClient::select(int timeout_sec, bool include_stdin) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);
  if (include_stdin) {
    FD_SET(STDIN_FILENO, &fds);
  }

  struct timeval tv;
  tv.tv_sec = timeout_sec;
  tv.tv_usec = 0;

  int n = ::select(sockfd + 1, &fds, NULL, NULL, &tv);
  return n;
}