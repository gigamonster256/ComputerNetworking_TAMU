#include <iostream>

#include "http/client.hpp"

using namespace http;

void usage(const char* progname) {
  std::cerr << "Usage: " << progname << " <ip to bind> <port>" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage(argv[0]);
  }

  char* ip = argv[1];
  int port = atoi(argv[2]);


  
  Client client;
  client.get("www.example.com", "/");
  return 0;
}