To implement the Simple Broadcast Chat Protocol (SBCP) client and server in C++, here’s an outline of the steps, focusing on the mandatory features without the bonus features. I’ll provide sample skeleton code for both the client and server.

### Key Points from the Protocol:
- The client joins the server using a `JOIN` message.
- Messages are broadcast to all connected clients using `FWD` messages from the server.
- I/O multiplexing with `select` is required for handling multiple clients.
- Server and clients should manage client disconnects and reuse usernames.

### SBCP Message Structure
- **Header:** Version, Type, Length
- **Attributes:** Payload data (e.g., username, chat messages)

### 1. **Server Implementation (C++)**

The server listens for multiple clients using `select` and sends messages to all connected clients.

```cpp
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <map>
#include <fcntl.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define PORT 12345
#define BUFFER_SIZE 1024

struct Client {
    int sockfd;
    std::string username;
};

// Broadcast to all connected clients
void broadcastMessage(const std::string &message, const std::vector<Client> &clients, int sender_fd) {
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
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
            new_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
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
```

### 2. **Client Implementation (C++)**

The client sends messages to the server and listens for forwarded messages from other clients.

```cpp
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

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
```

### Explanation:
1. **Server:**
   - Handles multiple clients using `select` to avoid blocking.
   - Broadcasts messages received from one client to all others.
   - Detects client disconnection and removes them.

2. **Client:**
   - Connects to the server and uses `select` to handle user input and server messages simultaneously.
   - Sends user input to the server and prints received messages.

This is a basic implementation of the SBCP protocol as described in the assignment without the bonus features.