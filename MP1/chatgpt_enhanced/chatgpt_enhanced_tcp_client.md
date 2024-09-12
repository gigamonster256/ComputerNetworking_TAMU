Here are several improvements to the TCP client code along with explanations for each change:

### 1. **Enhanced Error Handling with Exceptions**
   - **Original**: The client exits with `exit(EXIT_FAILURE)` on error.
   - **Improvement**: Use exceptions instead of `exit()` for more flexible error handling and to avoid terminating the entire application in case of an error. This allows the user of the class to catch and handle errors in a controlled manner.

   ```cpp
   TCPClient::TCPClient(const char *server, int port_no) {
       bool is_ipv6 = false;
       for (auto p = server; *p; p++) {
           if (*p == ':') {
               is_ipv6 = true;
               break;
           }
       }

       if (!is_ipv6) {
           snprintf(peer_ip_addr, INET6_ADDRSTRLEN, "::ffff:%s", server);
       } else {
           strncpy(peer_ip_addr, server, INET6_ADDRSTRLEN);
       }

       sockfd = socket(AF_INET6, SOCK_STREAM, 0);
       if (sockfd < 0) {
           throw std::runtime_error("Failed to create socket");
       }

       struct sockaddr_in6 addr;
       memset(&addr, 0, sizeof(addr));
       addr.sin6_family = AF_INET6;
       addr.sin6_port = htons(port_no);
       if (inet_pton(AF_INET6, peer_ip_addr, &addr.sin6_addr) <= 0) {
           close(sockfd);
           throw std::runtime_error("Failed to convert IP address");
       }

       if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
           close(sockfd);
           throw std::runtime_error("Failed to connect to server");
       }
   }
   ```

   - **Explanation**: Exceptions allow the client to propagate errors upward without immediately terminating the program. This makes it easier to implement recovery mechanisms or retry logic in higher-level code.

### 2. **Timeouts for `connect()` and `read()`/`write()`**
   - **Original**: The client doesn’t implement any connection or read/write timeouts, meaning it could hang indefinitely.
   - **Improvement**: Add support for setting socket timeouts for better control of client operations.

   ```cpp
   bool TCPClient::set_timeout(int sec) {
       struct timeval timeout;
       timeout.tv_sec = sec;
       timeout.tv_usec = 0;

       if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
           perror("TCPClient setsockopt (SO_RCVTIMEO)");
           return false;
       }

       if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
           perror("TCPClient setsockopt (SO_SNDTIMEO)");
           return false;
       }

       return true;
   }
   ```

   - **Explanation**: Adding timeouts for socket operations ensures that the client doesn’t hang forever while trying to connect or during I/O operations, which improves robustness.

### 3. **Graceful Handling of Partial Reads/Writes**
   - **Original**: The `readn()` and `writen()` functions assume that the socket will always transfer the complete message. However, in real-world networks, partial reads and writes may occur.
   - **Improvement**: Check for partial reads/writes and handle them properly in a loop until all data is read or written.

   ```cpp
   ssize_t TCPClient::writen(void *msgbuf, size_t len) {
       size_t total_written = 0;
       while (total_written < len) {
           ssize_t n_written = ::write(sockfd, (char *)msgbuf + total_written, len - total_written);
           if (n_written < 0) {
               if (errno == EINTR) {
                   continue; // interrupted by signal, retry
               }
               return n_written;
           }
           total_written += n_written;
       }
       return total_written;
   }

   void TCPClient::readn(void *msgbuf, size_t len) {
       size_t total_read = 0;
       while (total_read < len) {
           ssize_t n_read = ::read(sockfd, (char *)msgbuf + total_read, len - total_read);
           if (n_read < 0) {
               if (errno == EINTR) {
                   continue; // interrupted by signal, retry
               }
               throw std::runtime_error("TCPClient read error");
           } else if (n_read == 0) {
               throw std::runtime_error("TCPClient read EOF before finished");
           }
           total_read += n_read;
       }
   }
   ```

   - **Explanation**: This ensures that even if a `read()` or `write()` is interrupted or only partially completes, the client will continue until the requested amount of data has been transferred.

### 4. **Improved Buffer Handling in `readline()`**
   - **Original**: The `readline()` function reads one byte at a time, which is inefficient.
   - **Improvement**: Use a small buffer to minimize the number of `read()` system calls and improve performance.

   ```cpp
   size_t TCPClient::readline(void *msgbuf, size_t maxlen) {
       size_t n_read = 0;
       char buffer[1024]; // small buffer for efficient reads

       while (n_read < maxlen - 1) {
           ssize_t n = ::read(sockfd, buffer, sizeof(buffer));
           if (n < 0) {
               if (errno == EINTR) {
                   continue;
               }
               throw std::runtime_error("TCPClient read error");
           } else if (n == 0) {
               break; // EOF
           }
           for (ssize_t i = 0; i < n && n_read < maxlen - 1; i++) {
               ((char *)msgbuf)[n_read++] = buffer[i];
               if (buffer[i] == '\n') {
                   ((char *)msgbuf)[n_read] = '\0';
                   return n_read;
               }
           }
       }
       ((char *)msgbuf)[n_read] = '\0';
       return n_read;
   }
   ```

   - **Explanation**: Using a buffer reduces the number of system calls (`read()`) by reading multiple bytes at once, which improves performance, especially in high-latency environments.

### 5. **Use RAII for Socket Management**
   - **Original**: The socket is manually closed in the destructor, which can lead to resource leaks if not handled correctly.
   - **Improvement**: Use RAII (Resource Acquisition Is Initialization) to manage the socket lifecycle. This ensures that the socket is always closed, even if an exception is thrown.

   ```cpp
   TCPClient::TCPClient(TCPClient&& other) noexcept : sockfd(other.sockfd) {
       other.sockfd = -1; // take ownership of the socket
   }

   TCPClient& TCPClient::operator=(TCPClient&& other) noexcept {
       if (this != &other) {
           close(sockfd);
           sockfd = other.sockfd;
           other.sockfd = -1;
       }
       return *this;
   }

   TCPClient::~TCPClient() {
       if (sockfd >= 0) {
           close(sockfd);
       }
   }
   ```

   - **Explanation**: Using RAII ensures that the socket is always properly closed when the object goes out of scope. Adding move semantics (`move constructor` and `move assignment operator`) allows efficient transfer of resources, preventing double closure of the socket.

### 6. **Non-blocking Reads and Writes (Optional)**
   - **Original**: The client uses blocking I/O, which could cause the client to hang if the server doesn't respond.
   - **Improvement**: Optionally allow non-blocking reads and writes using `fcntl()`.

   ```cpp
   bool TCPClient::set_nonblocking() {
       int flags = fcntl(sockfd, F_GETFL, 0);
       if (flags == -1) {
           perror("TCPClient fcntl get");
           return false;
       }

       if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
           perror("TCPClient fcntl set");
           return false;
       }

       return true;
   }
   ```

   - **Explanation**: Non-blocking mode allows the client to perform asynchronous operations without blocking the main thread, which is useful for high-performance or event-driven applications.

### 7. **Support for IPv4 and IPv6 Transparently**
   - **Original**: The code checks if the address contains a colon (`:`) to determine whether it is IPv6. However, it assumes that if no colon is found, the address is IPv4, which may not always be reliable.
   - **Improvement**: Use `getaddrinfo()` to handle both IPv4 and IPv6 transparently.

   ```cpp
   TCPClient::TCPClient(const char *server, int port_no) {
       struct addrinfo hints, *res;
       memset(&hints, 0, sizeof(hints));
       hints.ai_family = AF_UNSPEC;  // Support both IPv4 and IPv6
       hints.ai_socktype = SOCK_STREAM;

       char port_str[6];
       snprintf(port_str, sizeof(port_str), "%d", port_no);

       if (getaddrinfo(server, port_str, &hints, &res) != 0) {
           throw std::runtime_error("Failed to resolve server address");
       }

       sockfd = socket(res->ai_family, res->