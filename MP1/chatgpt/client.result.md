Here’s an example of a C client program that connects to a TCP echo server according to RFC 862, which defines a simple Echo Protocol. The program allows the user to send data through standard input, which is sent to the echo server. The server responds by echoing the data, and the client prints the received data. The process loops until the user sends EOF (Ctrl+D) or terminates the client.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Server address
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Configure server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Convert server IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        error("ERROR invalid server IP address");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("ERROR connecting to the server");
    }

    printf("Connected to the echo server at %s:%d\n", server_ip, server_port);

    char buffer[BUFFER_SIZE];

    // Loop until EOF or error
    while (1) {
        printf("Enter message: ");

        // Read user input from stdin
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            // If fgets returns NULL, it means EOF was received
            printf("EOF received. Exiting...\n");
            break;
        }

        // Send data to the server
        int len = strlen(buffer);
        if (send(sockfd, buffer, len, 0) < 0) {
            error("ERROR sending data");
        }

        // Receive the echoed data back from the server
        int n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n < 0) {
            error("ERROR receiving data");
        }

        // Null-terminate the received data
        buffer[n] = '\0';

        // Print the echoed data
        printf("Echoed from server: %s", buffer);
    }

    // Close the socket
    close(sockfd);
    return 0;
}
```

### Key Points:
1. **Socket Creation:** The program uses `socket(AF_INET, SOCK_STREAM, 0)` to create a TCP socket.
2. **Server Connection:** It connects to the echo server using `connect()`.
3. **Data I/O:** The program reads input from `stdin` (using `fgets()`), sends it to the server, and receives the echoed response (via `send()` and `recv()`).
4. **Termination:** The program terminates if EOF is sent (Ctrl+D in terminal) or if any socket error occurs.

### Usage:
1. Compile the program:
   ```bash
   gcc -o echo_client echo_client.c
   ```
2. Run it, providing the server IP and port:
   ```bash
   ./echo_client 127.0.0.1 7
   ```
   Replace `127.0.0.1` with the IP of the echo server, and `7` with the port number where the echo server is listening (port 7 is the default for RFC 862 Echo servers).