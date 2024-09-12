#include "http/client.hpp"

int main() {
  http::Client client;
  client.get("neverssl.com", "/");
  return 0;
}