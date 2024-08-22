#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "common.hpp"

class TCPClient {
 private:
  ip_version version;
  int sockfd;
  char ip_addr[INET6_ADDRSTRLEN];

 public:
  // connect to server
  TCPClient(const char* server, int port_no);
  ~TCPClient();

  ssize_t writen(void* msgbuf, size_t len);
  ssize_t readline(void* msgbuf, size_t maxlen);
  ssize_t read(void* msgbuf, size_t len);

  const char* ip() const { return ip_addr; }

 private:
  // create a channel from an existing socket
  TCPClient(int sockfd, sockaddr_in6 client_addr);

  friend class TCPServer;
};

#endif