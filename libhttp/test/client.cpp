#include "http/client.hpp"

#include <iostream>

#include "http/message.hpp"

using namespace http;

int main() {
  Client client("neverssl.com");
  auto response = client.get("/");
  std::cout << "Status-Code: " << response.get_status_code() << std::endl;
  auto& headers = response.get_headers();
  std::cout << "Number of headers: " << headers.size() << std::endl;
  for (const auto& header : response.get_headers()) {
    std::cout << *header << std::endl;
  }
  std::cout << "Body: " << response.get_body().value_or("") << std::endl;
  return 0;
}