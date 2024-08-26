#include <iostream>
#include <sys/select.h>
#include <unistd.h>

#include "sbcp_messages.hpp"
#include "tcp_client.hpp"

using namespace sbcp;

void usage(const char *progname) {
  std::cerr << "Usage: " << progname << " <username> <server> <port>"
            << std::endl;
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    usage(argv[0]);
  }

  const char *username = argv[1];
  const char *server = argv[2];
  int port = atoi(argv[3]);

  TCPClient client(server, port);
  const message_t join_msg = JOIN(username);
  client.writen((void *)join_msg.data(), join_msg.size());
  // we should get an ACK message
  message_t server_response;
  client.readn((void *)server_response.data(), sizeof(message_t::header_t));
  client.readn((void *)(server_response.data() + sizeof(message_t::header_t)),
               server_response.get_length());
  if (server_response.get_type() == message_t::Type::ACK) {
    std::cout << "Connected to server" << std::endl;
    std::cout << "Client count: " << server_response.get_client_count() << std::endl;
    if (server_response.get_client_count() > 1) {
      std::cout << "Clients: ";
      for (const auto &username : server_response.get_usernames()) {
        std::cout << username << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << "No other clients connected" << std::endl;
    }
  } else {
    std::cerr << "Failed to connect to server" << std::endl;
    assert(server_response.get_type() == message_t::Type::NAK);
    std::cerr << "Reason: " << server_response.get_reason() << std::endl;
    exit(EXIT_FAILURE);
  }

  while(true) {
    // read from client or stdin
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client.get_fd(), &readfds);
    FD_SET(STDIN_FILENO, &readfds);
    int ret = select(client.get_fd() + 1, &readfds, NULL, NULL, NULL);
    if (ret < 0) {
      perror("select");
      exit(1);
    }
    message_t msg;
    std::memset((void*)msg.data(), 0, sizeof(message_t));
    if (FD_ISSET(client.get_fd(), &readfds)) {
      // read from server
      client.readn((void *)msg.data(), sizeof(message_t::header_t));
      client.readn((void *)(msg.data() + sizeof(message_t::header_t)),
                   msg.get_length());
      if (msg.get_type() == message_t::Type::FWD) {
        std::cout << msg.get_username() << ": " << msg.get_message() << std::endl;
      } else if (msg.get_type() == message_t::Type::OFFLINE) {
        std::cout << msg.get_username() << " is offline" << std::endl;
      } else if (msg.get_type() == message_t::Type::ONLINE) {
        std::cout << msg.get_username() << " is online" << std::endl;
      } else if (msg.get_type() == message_t::Type::IDLE) {
        std::cout << msg.get_username() << " is idle" << std::endl;
      }
    } else 
    // read from stdin
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      // read from stdin
      std::string message;
      std::getline(std::cin, message);
      if (message.empty()) {
        break;
      }
      const message_t msg = SEND(message);
      client.writen((void *)msg.data(), msg.size());
    }
  }
}