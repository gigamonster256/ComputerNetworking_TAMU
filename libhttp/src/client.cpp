#include "http/client.hpp"

#include <iostream>
#include <memory>

#include "http/constants.hpp"
#include "http/header.hpp"
#include "http/message.hpp"
#include "http/request-line.hpp"
#include "tcp/client.hpp"

namespace http {

std::unique_ptr<Message> Client::get(const std::string& rel_path,
                                     HeaderList&& additional_headers) {
  Message request = Message::GET(rel_path);
  request.add_header(Header::parse_header("Host", host));
  request.add_header(Header::parse_header("Connection", "close"));
  for (auto& header : additional_headers) {
    request.add_header(std::move(header));
  }

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
  return std::make_unique<Message>(response_str);
}

}  // namespace http