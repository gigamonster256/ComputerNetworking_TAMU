#pragma once

#include <memory>
#include <string>

#include "http/constants.hpp"
#include "http/message.hpp"

namespace http {

class Client {
  std::string host;
  int port;

 public:
  Client(const std::string& host) : host(host), port(HTTP_PORT) {}
  Client(const std::string& host, int port) : host(host), port(port) {}
  std::unique_ptr<Message> get(const std::string& path);
};

}  // namespace http
