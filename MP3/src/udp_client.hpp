#ifndef _UDP_CLIENT_HPP_
#define _UDP_CLIENT_HPP_

#include "udp_common.hpp"

class UDPClient {
 private:
  ip_version version;
  struct sockaddr_in6 peer_addr;
  int sockfd;
  char peer_ip_addr[INET6_ADDRSTRLEN];
  bool connected_to_ephemeral_port = false;

 public:
  // connect to server
  UDPClient(const char* server, int initial_port_no);
  ~UDPClient();

  // passthrough I/O (sets errno on error)
  ssize_t write(void* msgbuf, size_t maxlen);
  ssize_t read(void* msgbuf, size_t maxlen);

  // get the file descriptor
  int get_fd() const { return sockfd; }

  // ip address of the peer
  const char* peer_ip() const { return peer_ip_addr; }

 private:
  // connect back to client
  UDPClient(const struct sockaddr_in6& client_addr);
  void connect_to_ephemeral_port(const struct sockaddr_in6& server_addr);

  friend class UDPServer;
};

#endif