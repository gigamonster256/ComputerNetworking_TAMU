#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "sbcp/messages.hpp"
#include "tcp/server.hpp"

using namespace sbcp;

struct bootstrap_message {
  pid_t handler_pid;
  uint8_t username_length;
  char username[SBCP_MAX_USERNAME_LENGTH];
};

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
  try {
    message->validate();
  } catch (MessageException &e) {
    std::cerr << "Invalid message: " << e.what() << std::endl;
    return -1;
  }
  auto r2 = read(fd, (void *)(message->data() + sizeof(message_t::header_t)),
                 message->get_length());
  assert(r2 == message->get_length());
  return r + r2;
}

ssize_t read_message(tcp::Client *client, message_t *message) {
  return read_message(client->get_fd(), message);
}

// global temp dir set on server start
char *temp_dir;

std::string main_fifo_name(pid_t pid) {
  return temp_dir + std::string("/") + std::to_string(pid) + "_main";
}

std::string handler_fifo_name(pid_t pid) {
  return temp_dir + std::string("/") + std::to_string(pid) + "_handler";
}

void client_handler(tcp::Client *client, void *extra_data) {
  fprintf(stderr, "Handler started\n");
  std::cerr << "Peer: " << client->peer_ip() << std::endl;

  // extra_data is the pipe created in main
  // used for bootstrapping the handler - main communication channel
  int *fds = (int *)extra_data;
  int bootstrap_fd = fds[1];
  // close read end of pipe
  close(fds[0]);
  // fifos to talk to main thread
  int handler_fd = -1;
  int main_fd = -1;
  bool fifos_created = false;
  pid_t pid = getpid();

  while (true) {
    // select on client and main_fd
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client->get_fd(), &readfds);

    // only listen to main_fd if we have joined
    if (main_fd >= 0) {
      FD_SET(main_fd, &readfds);
    }
    int ready = select(std::max(client->get_fd(), main_fd) + 1, &readfds, NULL,
                       NULL, NULL);
    if (ready < 0) {
      perror("Handler select");
      if (errno == EINTR) {
        continue;
      }
      break;
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
        std::cerr << "Client disconnected" << std::endl;
        break;
      }
      std::cerr << "Handler received message from client" << std::endl;
      std::cerr << message << std::endl;

      switch (message.get_type()) {
        // joins bootstrap the handler
        case message_type_t::JOIN: {
          // make fifos
          auto main_fifo = main_fifo_name(pid);
          if (mkfifo(main_fifo.c_str(), 0666) < 0) {
            perror("Handler mkfifo main");
            return;
          }

          auto handler_fifo = handler_fifo_name(pid);
          if (mkfifo(handler_fifo.c_str(), 0666) < 0) {
            perror("Handler mkfifo handler");
            unlink(handler_fifo.c_str());
            return;
          }

          fifos_created = true;

          // send bootstrap message to main thread
          auto username = message.get_username();
          std::cerr << "Handler bootstrapping for " << username << std::endl;
          struct bootstrap_message bootstrap(
              {pid, (uint8_t)username.size(), {}});
          strcpy(bootstrap.username, username.c_str());
          ssize_t r = write(bootstrap_fd, (void *)&bootstrap, sizeof(bootstrap));
          if (r < 0) {
            perror("Handler write bootstrap");
            return;
          }
          if ((size_t)r != sizeof(bootstrap)) {
            std::cerr << "Handler wrote " << r << " bytes, expected "
                      << sizeof(bootstrap) << std::endl;
            return;
          }

          std::cerr << "Handler sent bootstrap message to main thread"
                    << std::endl;

          std::cerr << "Handler waiting for main thread to connect"
                    << std::endl;
          // open fifos
          main_fd = open(main_fifo.c_str(), O_RDONLY);
          if (main_fd < 0) {
            perror("Handler open main");
            unlink(main_fifo.c_str());
            unlink(handler_fifo.c_str());
            return;
          }

          handler_fd = open(handler_fifo.c_str(), O_WRONLY);
          if (handler_fd < 0) {
            perror("Handler open handler");
            close(main_fd);
            unlink(main_fifo.c_str());
            unlink(handler_fifo.c_str());
            return;
          }

          // close write end of pipe
          if (close(bootstrap_fd) < 0) {
            perror("Handler close bootstrap");
            close(main_fd);
            close(handler_fd);
            unlink(main_fifo.c_str());
            unlink(handler_fifo.c_str());
            return;
          }

          break;
        }
        // forward message to main thread
        case message_type_t::SEND:
        case message_type_t::IDLE: {
          if (!fifos_created) {
            std::cerr << "Handler not bootstrapped yet" << std::endl;
            break;
          }
          std::cerr << "Handler forwarding message to main thread" << std::endl;
          auto r = write(handler_fd, message.data(), message.size());
          if (r < 0) {
            perror("Handler write to main");
            continue;
          }
          if ((size_t)r != message.size()) {
            std::cerr << "Handler wrote " << r << " bytes, expected "
                      << message.size() << std::endl;
          }
          break;
        }
        default: {
          // other message types
          std::cerr << "Should not get message type: " << message.get_type()
                    << " from client" << std::endl;
          break;
        }
      }
    }

    // message from main thread
    else if (main_fd >= 0 && FD_ISSET(main_fd, &readfds)) {
      message_t message;
      auto r = read_message(main_fd, &message);
      if (r == 0) {
        // main thread disconnected
        std::cerr << "Main thread closed the fifo... I guess we aren't allowed "
                     "to talk anymore"
                  << std::endl;
        break;
      }
      if (r < 0) {
        std::cerr << "Handler read error from main thread" << std::endl;
        continue;
      }
      std::cerr << "Handler received message from main thread" << std::endl;
      std::cerr << message << std::endl;
      std::cerr << "Forwarding message to client" << std::endl;

      // send message to client
      client->write((void *)message.data(), message.size());
    } else {
      std::cerr << "Should not get here" << std::endl;
      assert(false);
    }
  }

  // cleanup
  if (handler_fd >= 0) {
    close(handler_fd);
  }
  if (main_fd >= 0) {
    close(main_fd);
  }
  if (fifos_created) {
    unlink(main_fifo_name(pid).c_str());
    unlink(handler_fifo_name(pid).c_str());
  }
  fprintf(stderr, "Handler finished\n");
}

void timeout_handler() {
  //   fprintf(stderr, "No client in the last timeout interval\n");
}

void write_all(std::map<std::string, int> &fifos, const message_t &message) {
  for (auto &pair : fifos) {
    ssize_t r = write(pair.second, message.data(), message.size());
    if (r < 0) {
      perror("write_all");
    }
    if ((size_t)r != message.size()) {
      std::cerr << "write_all wrote " << r << " bytes, expected "
                << message.size() << std::endl;
    }
  }
}

void write_all_except(std::map<std::string, int> &fifos,
                      const message_t &message, const std::string &username) {
  for (auto &pair : fifos) {
    if (pair.first != username) {
      ssize_t r = write(pair.second, message.data(), message.size());
      if (r < 0) {
        perror("write_all_except");
      }
      if ((size_t)r != message.size()) {
        std::cerr << "write_all_except wrote " << r << " bytes, expected "
                  << message.size() << std::endl;
      }
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
  unsigned int num_extra_clients = 2;
  //(void)ip;

  // clients --> server pipe
  // used for notifying server of new client
  int pipefd[2];
  int fd = pipe(pipefd);
  if (fd < 0) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // make temp dir
  char temp[] = "/tmp/sbcp_XXXXXX";
  temp_dir = mkdtemp(temp);
  if (temp_dir == NULL) {
    perror("mkdtemp");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "Temp dir: %s\n", temp_dir);
  std::cout << "IP Address provided:" << ip << std::endl;
  tcp::Server server;
  auto pid = server.set_port(port)
                 .set_ip_addr(ip)
                 .add_handler(client_handler)
                 .set_max_clients(max_clients + num_extra_clients)
                 .set_timeout_handler(timeout_handler)
                 // give write end of pipe to clients
                 .add_handler_extra_data((void *)pipefd)
                 //  .debug(true)
                 .start();
  std::cerr << "Server started with pid: " << pid << std::endl;

  // username -> fifo set
  std::map<std::string, int> rd_fifos;
  std::map<std::string, int> wr_fifos;

  // online users
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

    // client joining
    if (FD_ISSET(pipefd[0], &readfds)) {
      struct bootstrap_message message;
      auto n = read(pipefd[0], (void *)&message, sizeof(message));
      if (n == 0) {
        // server closed the pipe
        std::cerr << "Server closed the pipe... Something went seriously wrong"
                  << std::endl;
        break;
      }
      if (n != sizeof(message)) {
        std::cerr << "Main read " << n << " bootstrap bytes, expected "
                  << sizeof(message) << std::endl;
        continue;
      }
      std::cerr << "Main thread recieved bootstrap message from client"
                << std::endl;
      std::cerr << "Handler pid: " << message.handler_pid << std::endl;

      // establish fifo connection with handler
      auto main_fifo = main_fifo_name(message.handler_pid);
      int main_fd = open(main_fifo.c_str(), O_WRONLY);
      if (main_fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      auto handler_fifo = handler_fifo_name(message.handler_pid);
      int handler_fd = open(handler_fifo.c_str(), O_RDONLY);
      if (handler_fd < 0) {
        perror("open");
        close(main_fd);
        exit(EXIT_FAILURE);
      }

      // make sure username is unique
      std::string username(message.username, message.username_length);
      if (rd_fifos.find(username) != rd_fifos.end()) {
        std::cerr << "Username " << username << " already exists" << std::endl;
        // send nak message to handler thread
        message_t nak = NAK("Username already exists");
        ssize_t r = write(main_fd, nak.data(), nak.size());
        if (r < 0) {
          perror("write");
        }
        if ((size_t)r != nak.size()) {
          std::cerr << "Main wrote " << r << " bytes, expected " << nak.size()
                    << std::endl;
        }
        close(handler_fd);
        close(main_fd);
        continue;
      }

      // make sure max_clients limit is not crossed
      if (online_users.size() == max_clients) {
        std::cerr << "Maximum connected clients limit reached";
        // send NAK message to the handler thread with reason
        message_t nak = NAK("Maximum clients limit");
        ssize_t r = write(main_fd, nak.data(), nak.size());
        if (r < 0) {
          perror("write");
        }
        if ((size_t)r != nak.size()) {
          std::cerr << "Main wrote " << r << " bytes, expected " << nak.size()
                    << std::endl;
        }
        close(handler_fd);
        close(main_fd);
        continue;
      }

      std::cerr << "New user: " << username << std::endl;

      rd_fifos[username] = handler_fd;
      wr_fifos[username] = main_fd;
      auto other_users = online_users;
      online_users.push_back(username);

      // send ack message to handler thread
      message_t ack = ACK(online_users.size(), other_users);
      ssize_t r = write(main_fd, ack.data(), ack.size());
      if (r < 0) {
        perror("write");
      }
      if ((size_t)r != ack.size()) {
        std::cerr << "Main wrote " << r << " bytes, expected " << ack.size()
                  << std::endl;
      }
      std::cerr << "Sent ACK to handler thread" << std::endl;

      // send online message to all other clients
      message_t online = ONLINE(username);
      write_all_except(wr_fifos, online, username);
      std::cerr << "Sent ONLINE to all other clients" << std::endl;
    }
    // extablished clients
    else {
      for (auto &pair : rd_fifos) {
        if (FD_ISSET(pair.second, &readfds)) {
          auto username = pair.first;
          message_t message;
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
          std::cerr << "Main thread recieved fifo message from " << username
                    << std::endl;
          std::cerr << message << std::endl;
          switch (message.get_type()) {
            // forward message to all other clients
            case message_type_t::SEND: {
              message.change_to_fwd(username);
              write_all_except(wr_fifos, message, username);
              break;
            }
            // forward idle message to all other clients with the username
            case message_type_t::IDLE: {
              std::cerr << "Main thread recieved IDLE message from " << username
                        << std::endl;
              message.add_username(username.c_str());
              write_all_except(wr_fifos, message, username);
              break;
            }
            default: {
              // other message types
              std::cerr << "Should not get message type: " << message.get_type()
                        << " from client on fifo" << std::endl;
            }
          }
          break;
        }
      }
    }
  }

  // add way to close server?
  assert(false);

  // cleanup temp dir
  rmdir(temp_dir);

  waitpid(pid, nullptr, 0);
  close(pipefd[0]);
  close(pipefd[1]);
}
