#include "udp/server.hpp"

#include "tftp/packets.hpp"

#include <stdio.h>

#include <iostream>

using namespace udp;
using namespace tftp;

using ErrorCode = Packet::ErrorCode;

int main() {
  Server server;
  server.set_port(12345)
      .set_max_clients(5)
      .add_handler(
          [](Client* client, const char* msg, size_t len, client_data_ptr_t) {
            fprintf(stderr, "Received message: %s\n", msg);
            client->write((void*)msg, len);
          })
      .debug(true)
      .exec();
  return 0;
}