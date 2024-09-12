#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: echo <IPAdr> <Port>" << std::endl;
    return 1;
  }

  const char *server_ip = argv[1];
  int port = std::stoi(argv[2]);

  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    perror("Socket creation failed");
    return 1;
  }

  sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address or address not supported" << std::endl;
    return 1;
  }

  if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Connection failed");
    return 1;
  }

  std::cout << "Connected to server" << std::endl;

  char buffer[BUFFER_SIZE];
  while (true) {
    std::cout << "Enter message: ";
    if (!std::cin.getline(buffer, BUFFER_SIZE)) {
      break;  // EOF detected (e.g., Ctrl+D)
    }

    write(client_socket, buffer, strlen(buffer));
    ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';  // Null-terminate the received message
      std::cout << "Echo from server: " << buffer << std::endl;
    }
  }

  close(client_socket);  // Close the socket
  return 0;
}
