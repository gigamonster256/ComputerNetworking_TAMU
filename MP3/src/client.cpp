#include <stdio.h>

#include "udp_client.hpp"

int main() {
  UDPClient client("127.0.0.1", 12345);
  fprintf(stderr, "Connected to server\n");
  char message[] = "Hello, server!";
  client.write(message, sizeof(message));
  char buf[1024];
  size_t len = client.read(buf, sizeof(buf));
  buf[len] = '\0';
  printf("Received: %s\n", buf);
  printf("Length: %lu\n", len);
  printf("Client IP: %s\n", client.peer_ip());
  return 0;
}