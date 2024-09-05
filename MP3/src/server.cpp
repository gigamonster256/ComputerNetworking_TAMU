#include <stdio.h>

#include "udp_server.hpp"

int main() {
  UDPServer server;
  server.set_port(12345)
      .set_max_clients(5)
      .add_handler([](UDPClient* client, const char* msg, size_t len,
                      client_data_ptr_t) {
        fprintf(stderr, "Received message: %s\n", msg);
        client->write((void*)msg, len);
      })
      .debug(true)
      .exec();
  return 0;
}