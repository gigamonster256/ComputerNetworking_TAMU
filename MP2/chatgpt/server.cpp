#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <set>
#include <vector>

#define MAX_CLIENTS 10
#define PORT 12345
#define BUFFER_SIZE 1024

struct Client {
  int sockfd;
  std::string username;
};

// Broadcast to all connected clients
void broadcastMessage(const std::string &message,
                      const std::vector<Client> &clients, int sender_fd) {
  for (const auto &client : clients) {
    if (client.sockfd != sender_fd) {
      send(client.sockfd, message.c_str(), message.size(), 0);
    }
  }
}

int main() {
  int server_sock, new_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  // Create server socket
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  // Set socket options
  int opt = 1;
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Bind server socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);
  if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(server_sock);
    return 1;
  }

  // Listen for incoming connections
  if (listen(server_sock, MAX_CLIENTS) < 0) {
    perror("Listen failed");
    close(server_sock);
    return 1;
  }

  // Prepare for select()
  fd_set read_fds;
  int max_fd = server_sock;
  std::vector<Client> clients;

  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(server_sock, &read_fds);

    for (const auto &client : clients) {
      FD_SET(client.sockfd, &read_fds);
      if (client.sockfd > max_fd) {
        max_fd = client.sockfd;
      }
    }

    // Select system call to multiplex I/O
    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
      perror("Select error");
      return 1;
    }

    // Handle new connections
    if (FD_ISSET(server_sock, &read_fds)) {
      new_sock =
          accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
      if (new_sock < 0) {
        perror("Accept error");
        continue;
      }
      // Add the new client to the list
      Client new_client = {new_sock, ""};
      clients.push_back(new_client);
    }

    // Handle data from clients
    for (auto it = clients.begin(); it != clients.end();) {
      if (FD_ISSET(it->sockfd, &read_fds)) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(it->sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
          // Client disconnected
          close(it->sockfd);
          it = clients.erase(it);
        } else {
          // Broadcast message to other clients
          broadcastMessage(buffer, clients, it->sockfd);
          ++it;
        }
      } else {
        ++it;
      }
    }
  }

  close(server_sock);
  return 0;
}
