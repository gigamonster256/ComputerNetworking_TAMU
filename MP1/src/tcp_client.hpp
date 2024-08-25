#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_

#include "common.hpp"

class TCPClient {
 private:
  ip_version version;
  int sockfd;
  char peer_ip_addr[INET6_ADDRSTRLEN];

 public:
  // connect to server
  TCPClient(const char* server, int port_no);
  ~TCPClient();

  // simple I/O (crashes on error)
  size_t writen(void* msgbuf, size_t len);
  size_t readn(void* msgbuf, size_t len);
  size_t readline(void* msgbuf, size_t maxlen);

  // passthrough I/O (sets errno on error)
  ssize_t write(void* msgbuf, size_t maxlen);
  ssize_t read(void* msgbuf, size_t maxlen);

  // fancy I/O (wraps select)
  ssize_t readn_timeout(void* msgbuf, size_t len, int timeout_sec);
  ssize_t readline_timeout(void* msgbuf, size_t maxlen, int timeout_sec);
  ssize_t read_timeout(void* msgbuf, size_t maxlen, int timeout_sec);

  // fancy select
  int select(int timeout_sec, bool include_stdin = false);

  // ip address of the peer
  const char* peer_ip() const { return peer_ip_addr; }

 private:
  // create a channel from an existing socket
  TCPClient(int sockfd, sockaddr_in6 client_addr);

  friend class TCPServer;
};

#endif