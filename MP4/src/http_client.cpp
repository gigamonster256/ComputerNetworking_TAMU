#include <algorithm>
#include <fstream>
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

  std::cout << "Response status: " << response->get_status_code().to_string()
            << std::endl;
  std::cout << "Response headers:" << std::endl;
  for (const auto& header : response->get_headers()) {
    std::cout << *header << std::endl;
  }
  if (response->get_body()) {
    std::cout << "Response body:" << std::endl;
    std::cout << *response->get_body() << std::endl;
  }

  // save to file in write mode
  std::string filename = std::string(argv[3]);
  std::replace(filename.begin(), filename.end(), '/', '_');
  std::ofstream file("./" + filename + ".http");
  file << *response;
  file.close();

  return 0;
}