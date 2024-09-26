#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
  int sock;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  // Set server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  // Connect to server
  if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Connect failed");
    close(sock);
    return 1;
  }

  // Set up for select()
  fd_set read_fds;
  int max_fd = sock;

  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
      perror("Select error");
      return 1;
    }

    // Handle server messages
    if (FD_ISSET(sock, &read_fds)) {
      memset(buffer, 0, BUFFER_SIZE);
      int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
      if (bytes_received <= 0) {
        // Server disconnected
        std::cout << "Disconnected from server" << std::endl;
        break;
      }
      std::cout << "Message from server: " << buffer << std::endl;
    }

    // Handle user input
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      memset(buffer, 0, BUFFER_SIZE);
      read(STDIN_FILENO, buffer, BUFFER_SIZE);
      send(sock, buffer, strlen(buffer), 0);
    }
  }

  close(sock);
  return 0;
}
