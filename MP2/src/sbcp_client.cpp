#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include <iostream>

#include "sbcp/messages.hpp"
#include "tcp/client.hpp"

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

  // connect to server
  tcp::Client client(server, port);

  // send JOIN message
  const message_t join_msg = JOIN(username);
  client.writen((void *)join_msg.data(), join_msg.size());
  // we should get an ACK message
  // or a NAK message if the username is in use
  message_t server_response;
  client.readn((void *)server_response.data(), sizeof(message_t::header_t));
  client.readn((void *)(server_response.data() + sizeof(message_t::header_t)),
               server_response.get_length());

  // we're connected
  if (server_response.get_type() == message_t::Type::ACK) {
    std::cout << "Connected to server" << std::endl;
    std::cout << "Client count: " << server_response.get_client_count()
              << std::endl;
    if (server_response.get_client_count() > 1) {
      std::cout << "Clients: ";
      for (const auto &username : server_response.get_usernames()) {
        std::cout << username << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << "No other clients connected" << std::endl;
    }
  }
  // NAK (username in use)
  else {
    std::cerr << "Failed to connect to server" << std::endl;
    assert(server_response.get_type() == message_t::Type::NAK);
    std::cerr << "Reason: " << server_response.get_reason() << std::endl;
    exit(EXIT_FAILURE);
  }

  // get current timestamp
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);

  bool idle = false;
  bool displayed_prompt = false;

  // main loop
  while (true) {
    if (!displayed_prompt) {
      std::cout << "> " << std::flush;
      displayed_prompt = true;
    }
    // read from client or stdin
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client.get_fd(), &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval select_timeout;
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    // wait 1 second for input
    int ret =
        select(client.get_fd() + 1, &readfds, NULL, NULL, &select_timeout);
    if (ret < 0) {
      perror("select");
      exit(1);
    }

    // see if we've been idle for 10 seconds
    if (ret == 0) {
      // std::cerr << "Timeout" << std::endl;
      // we're already idle
      if (idle) {
        continue;
      }
      // check if we've been idle for 10 seconds
      struct timespec new_spec;
      clock_gettime(CLOCK_MONOTONIC, &new_spec);
      // std::cout << "Elapsed time: " << new_spec.tv_sec - spec.tv_sec <<
      // std::endl;
      if (new_spec.tv_sec - spec.tv_sec >= 10) {
        // std::cerr << "Idle for 10 seconds" << std::endl;
        // send IDLE message
        const message_t idle_msg = IDLE();
        client.writen((void *)idle_msg.data(), idle_msg.size());
        idle = true;
      }
      continue;
    }

    message_t msg;
    if (FD_ISSET(client.get_fd(), &readfds)) {
      // read from server
      client.readn((void *)msg.data(), sizeof(message_t::header_t));
      try {
        msg.validate();
      } catch (const MessageException &e) {
        std::cerr << e.what() << std::endl;
        continue;
      }
      client.readn((void *)(msg.data() + sizeof(message_t::header_t)),
                   msg.get_length());
      if (msg.get_type() == message_t::Type::FWD) {
        std::cout << msg.get_username() << ": " << msg.get_message()
                  << std::endl;
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
        // not idle anymore
        idle = false;
        clock_gettime(CLOCK_MONOTONIC, &spec);

        // read from stdin
        std::string message;
        std::getline(std::cin, message);
        if (message.empty()) {
          break;
        }
        const message_t msg = SEND(message);
        client.writen((void *)msg.data(), msg.size());
      }
      displayed_prompt = false;
  }
}