#include "http/client.hpp"

#include <iostream>

#include "http/message.hpp"

using namespace http;

int main() {
  Client client("neverssl.com");
  auto response = client.get("/");
  auto& headers = response.get_headers();
  std::cout << "Number of headers: " << headers.size() << std::endl;
  for (const auto& header : response.get_headers()) {
    std::cout << *header << std::endl;
  }
  return 0;
}