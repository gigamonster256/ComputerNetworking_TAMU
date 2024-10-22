#include <iostream>

#include "http/client.hpp"
#include "http/message.hpp"

using namespace http;

void usage(const char* progname) {
  std::cerr << "Usage: " << progname << " <proxy address> <proxy port> <url>"
            << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    usage(argv[0]);
  }

  Client client(argv[1], std::stoi(argv[2]));
  auto response = client.get(argv[3]);

  std::cout << *response << std::endl;

  return 0;
}