#include "client.hpp"

using namespace http;

int main() {
  Client client;
  client.get("neverssl.com", "/");
  return 0;
}