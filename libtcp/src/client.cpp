#include "tcp/client.hpp"

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace tcp {

Client::Client(int sockfd, sockaddr_in6 client_addr) : sockfd(sockfd) {
  if (inet_ntop(AF_INET6, &client_addr.sin6_addr, peer_ip_addr,
                sizeof(peer_ip_addr)) == NULL) {
    perror("TCPClient inet_ntop");
  }
}

Client::Client(const char *server, int port_no) {
  // check if a hostname or ip address was provided
  if (server == nullptr) {
    fprintf(stderr, "TCPClient: no server provided\n");
    exit(EXIT_FAILURE);
  }

  // check for any colons in the server address (IPv6)
  bool is_ipv6 = false;
  for (auto p = server; *p; p++) {
    if (*p == ':') {
      is_ipv6 = true;
      break;
    }
  }
  // check for any alphabetic characters in the server address
  bool is_hostname = false;
  if (!is_ipv6) {
    for (auto p = server; *p; p++) {
      if (isalpha(*p)) {
        is_hostname = true;
        break;
      }
    }
  }

  // get the address info if a hostname was provided
  if (is_hostname) {
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    int s = getaddrinfo(server, nullptr, &hints, &result);
    if (s != 0) {
      fprintf(stderr, "TCPClient getaddrinfo: %s\n", gai_strerror(s));
      exit(EXIT_FAILURE);
    }
    // set the peer ip address to the first address in the list
    // for now, only support IPv4
    char namebuf[INET_ADDRSTRLEN];
    if (result->ai_family == AF_INET) {
      if (inet_ntop(AF_INET,
                    &((struct sockaddr_in *)result->ai_addr)->sin_addr,
                    namebuf, sizeof(namebuf)) == NULL) {
        perror("TCPClient inet_ntop");
      }
      snprintf(peer_ip_addr, INET6_ADDRSTRLEN, "::ffff:%s", namebuf);
    } else {
      fprintf(stderr, "TCPClient: unsupported address family\n");
      exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
  }
  // ip address was provided
  else {
    // convert to v6 if it is v4
    if (!is_ipv6) {
      snprintf(peer_ip_addr, INET6_ADDRSTRLEN, "::ffff:%s", server);
    } else {
      strncpy(peer_ip_addr, server, INET6_ADDRSTRLEN);
      peer_ip_addr[INET6_ADDRSTRLEN - 1] = '\0';
    }
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
  if (inet_pton(AF_INET6, peer_ip_addr, &addr.sin6_addr) < 0) {
    perror("TCPClient inet_pton");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("TCPClient connect");
    exit(EXIT_FAILURE);
  }
}

Client::~Client() {
  if (close(sockfd) < 0) {
    perror("TCPClient close");
  }
}

ssize_t Client::writen(void *msgbuf, size_t len) {
  size_t n = 0;
  while (n < len) {
    ssize_t n_written = ::write(sockfd, (char *)msgbuf + n, len - n);
    if (n_written < 0) {
      if (errno == EINTR) {
        continue;
      }
      return n_written;
    }
    n += n_written;
  }
  return n;
}

void Client::readn(void *msgbuf, size_t len) {
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
      fprintf(stderr, "TCPClient read EOF before finished\n");
      exit(EXIT_FAILURE);
    }
    n += n_read;
  }
}

size_t Client::readline(void *msgbuf, size_t maxlen) {
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
  ((char *)msgbuf)[n_read] = '\0';
  return n_read;
}

ssize_t Client::write(void *msgbuf, size_t maxlen) {
  return ::write(sockfd, msgbuf, maxlen);
}

ssize_t Client::read(void *msgbuf, size_t maxlen) {
  return ::read(sockfd, msgbuf, maxlen);
}

}  // namespace tcp