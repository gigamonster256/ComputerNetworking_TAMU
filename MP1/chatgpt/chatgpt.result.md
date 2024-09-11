To implement the echo server and client as described in the document, here’s a basic version of both in C++. The server uses `fork()` to handle multiple clients, and the client sends and receives messages from the server.

### Echo Server (echos.cpp):

```cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        write(client_socket, buffer, bytes_read);  // Echo the message back
    }

    close(client_socket);  // Close the socket after the client disconnects
    exit(0);  // End the child process
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

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

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
```

### Echo Client (echo.cpp):

```cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
```

### Compilation:
You can compile both programs with:

```bash
g++ echos.cpp -o echos
g++ echo.cpp -o echo
```

### Execution:

1. Run the server:
   ```bash
   ./echos 8080
   ```

2. Run the client (in a different terminal):
   ```bash
   ./echo 127.0.0.1 8080
   ```

The client sends messages, and the server echoes them back. Multiple clients can connect simultaneously, and each will be handled by a child process.