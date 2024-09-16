#include "http/client.hpp"

#include "http/constants.hpp"
#include "tcp/client.hpp"

namespace http {

void Client::get(std::string host, std::string path) {
  std::string request = "GET ";
  request += path;
  request += " HTTP/1.0";
  request += CRLF;
  request += "Host: ";
  request += host;
  request += CRLF;
  request += "Connection: close";
  request += CRLF;
  request += CRLF;

  printf("Request:\n%s\n", request.c_str());
  tcp::Client client(host.c_str(), 80);
  client.writen((void*)request.c_str(), request.size());

  printf("Response:\n");
  char buffer[1024];
  // readline until we get an empty line
  while (client.readline(buffer, sizeof(buffer)) > 0) {
    printf("%s", buffer);
  }

  return;
}

}  // namespace http