#include <stdio.h>

#include "udp_server.hpp"

int main() {
  UDPServer server;
  server.set_port(12345)
      .set_max_clients(5)
      .add_handler([](UDPClient* client, client_data_ptr_t) {
        char buf[1024];
        fprintf(stderr, "Handling connection from %s\n", client->peer_ip());
        size_t len = client->read(buf, sizeof(buf));
        if (len == 0) {
          return;
        }
        buf[len] = '\0';
        printf("Received: %s\n", buf);
        client->writen(buf, len);
      })
      //   .debug(true)
      .exec();
  return 0;
}