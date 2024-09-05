#ifndef _UDP_CLIENT_HPP_
#define _UDP_CLIENT_HPP_

#include "udp_common.hpp"

class UDPClient {
 private:
  ip_version version;
  struct sockaddr_in6 peer_addr;
  int sockfd;
  char peer_ip_addr[INET6_ADDRSTRLEN];

 public:
  // connect to server
  UDPClient(const char* server, int initial_port_no);
  ~UDPClient();

  // simple I/O (crashes on error)
  size_t writen(void* msgbuf, size_t len);
  size_t readn(void* msgbuf, size_t len);
  size_t readline(void* msgbuf, size_t maxlen);

  // passthrough I/O (sets errno on error)
  ssize_t write(void* msgbuf, size_t maxlen);
  ssize_t read(void* msgbuf, size_t maxlen);

  // get the file descriptor
  int get_fd() const { return sockfd; }

  // ip address of the peer
  const char* peer_ip() const { return peer_ip_addr; }

 private:
  // connect back to client
  UDPClient(struct sockaddr_in6 client_addr);

  friend class UDPServer;
};

#endif