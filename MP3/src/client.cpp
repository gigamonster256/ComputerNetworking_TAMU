#include <stdio.h>

#include "udp_client.hpp"

int main() {
  UDPClient client("127.0.0.1", 12345);
  fprintf(stderr, "Connected to server\n");
  client.writen((void*)"Hello, world!", 13);
  char buf[1024];
  size_t len = client.read(buf, sizeof(buf));
  buf[len] = '\0';
  printf("Received: %s\n", buf);
  return 0;
}