#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <map>
#include <vector>

#include "sbcp_messages.hpp"
#include "tcp_server.hpp"

using namespace sbcp;

void usage(const char *progname) {
  std::cerr << "Usage: " << progname << " <ip> <port> <max_clients>"
            << std::endl;
  exit(EXIT_FAILURE);
}

ssize_t read_message(int fd, message_t *message) {
  auto r = read(fd, (void *)message->data(), sizeof(message_t::header_t));
  if (r == 0) {
    return 0;
  }
  assert(r == sizeof(message_t::header_t));
  message->validate();
  auto r2 = read(fd, (void *)(message->data() + sizeof(message_t::header_t)),
                 message->get_length());
  assert(r2 == message->get_length());
  return r + r2;
}

ssize_t read_message(TCPClient *client, message_t *message) {
  return read_message(client->get_fd(), message);
}

std::string publish_fifo_name(const std::string &username) {
  return username + "_publish";
}

std::string subscribe_fifo_name(const std::string &username) {
  return username + "_subscribe";
}

void client_handler(TCPClient *client, void *extra_data) {
  // extra_data is the write end of the main thread's pipe
  int main_fd = *((int *)extra_data);
  int pub_fd = -1;
  int sub_fd = -1;
  std::string username;

  fprintf(stderr, "Client connected\n");

  while (true) {
    // select on client and sub_fd
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client->get_fd(), &readfds);
    if (sub_fd >= 0) {
      FD_SET(sub_fd, &readfds);
    }
    int ready = select(std::max(client->get_fd(), sub_fd) + 1, &readfds, NULL,
                       NULL, NULL);
    if (ready < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (ready == 0) {
      continue;
    }

    // message from chat client
    if (FD_ISSET(client->get_fd(), &readfds)) {
      message_t message;
      auto r = read_message(client, &message);
      if (r == 0) {
        // client disconnected
        break;
      }
      std::cerr << "Handler received message from client" << std::endl;
      std::cerr << message << std::endl;

      switch (message.get_type()) {
        case message_type_t::JOIN: {
          username = message.get_username();

          // check to see if username is already taken
          // we dont have a way for the main thread to tell us this
          // so we just check if a file with the username exists
          auto pub_fifo_name = publish_fifo_name(username);
          if (access(pub_fifo_name.c_str(), F_OK) != -1) {
            std::cerr << "Username " << username << " already taken"
                      << std::endl;
            message_t nack = NAK("Username already taken");
            client->write((void *)nack.data(), nack.size());
            username.clear();
            break;
          }

          // make pub sub fifos
          if (mkfifo(pub_fifo_name.c_str(), 0666) < 0) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
          }
          auto sub_fifo_name = subscribe_fifo_name(username);
          if (mkfifo(sub_fifo_name.c_str(), 0666) < 0) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
          }
          // send join message to main thread
          write(main_fd, message.data(), message.size());

          // open pub sub fifos
          pub_fd = open(pub_fifo_name.c_str(), O_WRONLY);
          if (pub_fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
          }

          sub_fd = open(sub_fifo_name.c_str(), O_RDONLY);
          if (sub_fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
          }

          break;
        }
        case message_type_t::SEND:
        case message_type_t::IDLE: {
          assert(pub_fd >= 0);
          write(pub_fd, message.data(), message.size());
          break;
        }
        default: {
          // other message types
          std::cerr << "Should not get message type: " << message.get_type()
                    << " from client" << std::endl;
          assert(false);
          break;
        }
      }
    }
    // message from main thread
    else if (sub_fd >= 0 && FD_ISSET(sub_fd, &readfds)) {
      message_t message;
      read_message(sub_fd, &message);
      std::cerr << "Handler received message from main thread" << std::endl;
      std::cerr << message << std::endl;
      std::cerr << "Forwarding message to client" << std::endl;

      // send message to client
      client->write((void *)message.data(), message.size());
    }
  }
  if (pub_fd >= 0) {
    close(pub_fd);
  }
  if (sub_fd >= 0) {
    close(sub_fd);
  }
  if (!username.empty()) {
    unlink(publish_fifo_name(username).c_str());
    unlink(subscribe_fifo_name(username).c_str());
  }
  fprintf(stderr, "Client disconnected\n");
}

void timeout_handler() {
  //   fprintf(stderr, "No client in the last timeout interval\n");
}

void write_all(std::map<std::string, int> &fifos, message_t &message) {
  for (auto &pair : fifos) {
    write(pair.second, message.data(), message.size());
  }
}

void write_all_except(std::map<std::string, int> &fifos, message_t &message,
                      const std::string &username) {
  for (auto &pair : fifos) {
    if (pair.first != username) {
      write(pair.second, message.data(), message.size());
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    usage(argv[0]);
  }

  const char *ip = argv[1];
  unsigned int port = atoi(argv[2]);
  unsigned int max_clients = atoi(argv[3]);
  (void)ip;

  // clients --> server pipe
  // used for notifying server of new client
  int pipefd[2];
  int fd = pipe(pipefd);
  if (fd < 0) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  TCPServer server;
  auto pid = server.set_port(port)
                 .add_handler(client_handler)
                 .set_max_clients(max_clients)
                 .set_timeout_handler(timeout_handler)
                 // give write end of pipe to clients
                 .add_client_extra_data((void *)&pipefd[1])
                 //  .debug(true)
                 .start();
  std::cerr << "Server started with pid: " << pid << std::endl;

  // username -> fifo set
  std::map<std::string, int> rd_fifos;
  std::map<std::string, int> wr_fifos;
  std::vector<std::string> online_users;

  // get join messages from clients
  while (true) {
    // listen to pipe and clients
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipefd[0], &readfds);
    int max_fd = pipefd[0];
    for (auto &pair : rd_fifos) {
      FD_SET(pair.second, &readfds);
      max_fd = std::max(max_fd, pair.second);
    }
    int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (ready < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (ready == 0) {
      continue;
    }

    if (FD_ISSET(pipefd[0], &readfds)) {
      message_t message;
      read_message(pipefd[0], &message);
      std::cerr << "Main thread recieved bootstrap message from client"
                << std::endl;
      std::cerr << message << std::endl;

      switch (message.get_type()) {
        case message_type_t::JOIN: {
          std::string username = message.get_username();

          std::cerr << "Main thread bootstrapping handler for " << username
                    << std::endl;
          // the handler makes the pub sub fifos
          auto pub_fifo_name = publish_fifo_name(username);
          int pub_fd = open(pub_fifo_name.c_str(), O_RDONLY);
          if (pub_fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
          }
          auto sub_fifo_name = subscribe_fifo_name(username);
          int sub_fd = open(sub_fifo_name.c_str(), O_WRONLY);
          if (sub_fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
          }
          rd_fifos[username] = pub_fd;
          wr_fifos[username] = sub_fd;
          std::cerr << "User " << username << " joining room" << std::endl;
          auto other_users = online_users;
          online_users.push_back(username);
          message_t ack = ACK(other_users.size() + 1, other_users);
          write(wr_fifos[username], ack.data(), ack.size());
          std::cerr << "Sent ACK to handler thread" << std::endl;

          // send online message to all other clients
          message_t online = ONLINE(username);
          write_all_except(wr_fifos, online, username);
          std::cerr << "Sent ONLINE to all other clients" << std::endl;
          break;
        }
        default: {
          // other message types
          std::cerr << "Should not get message type: " << message.get_type()
                    << " from client on main pipe" << std::endl;
        }
      }
    } else {
      for (auto &pair : rd_fifos) {
        if (FD_ISSET(pair.second, &readfds)) {
          message_t message;
          memset((void *)message.data(), 0, sizeof(message_t));
          auto username = pair.first;
          auto r = read_message(pair.second, &message);
          // client disconnected
          if (r == 0) {
            std::cerr << "Handler for " << username << " disconnected"
                      << std::endl;
            close(pair.second);
            close(wr_fifos[username]);
            online_users.erase(
                std::remove(online_users.begin(), online_users.end(), username),
                online_users.end());
            wr_fifos.erase(username);
            rd_fifos.erase(username);

            // send offline message to all other clients
            message_t offline = OFFLINE(username);
            write_all(wr_fifos, offline);
            break;
          }
          std::cerr << "Main thread recieved fifo message from client"
                    << std::endl;
          std::cerr << message << std::endl;
          switch (message.get_type()) {
            case message_type_t::SEND: {
              message.change_to_fwd(username);
              write_all_except(wr_fifos, message, username);
              break;
            }
            case message_type_t::IDLE: {
              std::cerr << "Main thread recieved IDLE message from " << username
                        << std::endl;
              // send message to all other clients
              write_all_except(wr_fifos, message, username);
              break;
            }
            default: {
              // other message types
              std::cerr << "Should not get message type: " << message.get_type()
                        << " from client on fifo" << std::endl;
            }
          }
        }
      }
    }
  }

  waitpid(pid, nullptr, 0);
  close(pipefd[1]);
}
