#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;

  while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
    write(client_socket, buffer, bytes_read);  // Echo the message back
  }

  close(client_socket);  // Close the socket after the client disconnects
  exit(0);               // End the child process
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: echos <Port>" << std::endl;
    return 1;
  }

  int port = std::stoi(argv[1]);
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) {
    perror("Socket creation failed");
    return 1;
  }

  sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    return 1;
  }

  if (listen(server_socket, 10) < 0) {
    perror("Listen failed");
    return 1;
  }

  std::cout << "Server listening on port " << port << std::endl;

  while (true) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket =
        accept(server_socket, (sockaddr *)&client_addr, &client_len);

    if (client_socket < 0) {
      perror("Accept failed");
      continue;
    }

    if (fork() == 0) {
      close(server_socket);  // Close the server socket in the child process
      handle_client(client_socket);
    } else {
      close(client_socket);  // Parent process closes the client socket
    }
  }

  close(server_socket);  // Close the server socket
  return 0;
}
