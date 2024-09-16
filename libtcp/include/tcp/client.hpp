#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_

#include <arpa/inet.h>

namespace tcp {

class Client {
 private:
  int sockfd;
  char peer_ip_addr[INET6_ADDRSTRLEN];

 public:
  // connect to server
  Client(const char* server, int port_no);
  ~Client();

  // simple I/O (crashes on error)
  void readn(void* msgbuf, size_t len);

  // block until all bytes are written
  ssize_t writen(void* msgbuf, size_t len);

  // read a line of text (or up to maxlen - 1 bytes)
  size_t readline(void* msgbuf, size_t maxlen);

  // passthrough I/O (sets errno on error)
  ssize_t write(void* msgbuf, size_t maxlen);
  ssize_t read(void* msgbuf, size_t maxlen);

  // get the file descriptor
  int get_fd() const { return sockfd; }

  // ip address of the peer
  const char* peer_ip() const { return peer_ip_addr; }

 private:
  // create a channel from an existing socket
  Client(int sockfd, sockaddr_in6 client_addr);

  friend class Server;
};

}  // namespace tcp

#endif