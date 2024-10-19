#include "http/client.hpp"

#include <iostream>
#include <memory>

#include "http/constants.hpp"
#include "http/header.hpp"
#include "http/message.hpp"
#include "http/request-line.hpp"
#include "tcp/client.hpp"

namespace http {

Message Client::get(const std::string& rel_path) {
  auto headers = std::make_unique<HeaderList>();
  headers->push_back(Header::parse_header("Host", host));
  headers->push_back(Header::parse_header("Connection", "close"));
  Message request = Message::GET(rel_path);
  request.set_headers(std::move(headers));

  std::cout << "Request:\n" << request << std::endl;
  tcp::Client client(host.c_str(), port);
  std::string request_str = request.to_string();
  client.writen((void*)request_str.c_str(), request_str.size());

  std::string response_str;
  char buffer[1024];
  while (true) {
    ssize_t n = client.read(buffer, sizeof(buffer));
    if (n <= 0) {
      break;
    }
    response_str.append(buffer, n);
  }
  return Message(response_str);
}

}  // namespace http