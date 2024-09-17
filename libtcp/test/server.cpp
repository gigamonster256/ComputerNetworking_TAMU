#include "tcp/server.hpp"

#include <iostream>

#include "tcp/error.hpp"

using namespace tcp;

int main() {
  Server server;
  try {
    server
        .add_handler([](Client*, client_data_ptr_t) {
          std::cout << "Hello, world!" << std::endl;
        })
        .set_port(8080)
        .start();
  } catch (ConfigurationError& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  server.stop();

  return 0;
}