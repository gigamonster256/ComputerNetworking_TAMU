Here are several improvements made to the given TCP server code along with an explanation for each:

### 1. **Graceful Shutdown for Server**
   - **Original**: In the `stop(bool force)` function, the server stops either with `SIGTERM` or `SIGKILL`. However, if forceful shutdown (`SIGKILL`) is used, it abruptly kills the server without allowing it to clean up properly.
   - **Improvement**: Introduce a mechanism to first try shutting down gracefully with `SIGTERM`, and wait a few seconds for clients to terminate. If that fails, then send `SIGKILL`.

   ```cpp
   void TCPServer::stop(bool force) {
       if (server_pid < 0) {
           return;
       }
   
       if (!force) {
           if (kill(server_pid, SIGTERM) < 0) {
               perror("TCPServer kill");
           }
           // Wait for a few seconds to give time for graceful shutdown
           sleep(3);
           if (waitpid(server_pid, nullptr, WNOHANG) == 0) {
               force = true; // fallback to force if graceful fails
           }
       }

       if (force) {
           if (kill(server_pid, SIGKILL) < 0) {
               perror("TCPServer force kill");
           }
       }

       if (waitpid(server_pid, nullptr, 0) < 0) {
           perror("TCPServer waitpid");
       }

       server_pid = -1;
   }
   ```

   - **Explanation**: This allows the server to attempt a graceful shutdown, giving clients a chance to finish their operations. If the graceful shutdown fails after a timeout, it forcefully terminates the server.

### 2. **Error Handling for `fork()` in `start()`**
   - **Original**: `start()` immediately exits with an error if `fork()` fails.
   - **Improvement**: Handle `fork()` errors with retries or clearer logging, and make it non-blocking to avoid halting the server on fork failure.

   ```cpp
   pid_t TCPServer::start() {
       assert(server_pid < 0);
       assert(port_no > 0);
       assert(backlog > 0);
       assert(client_handler.max_clients > 0);
       assert(client_handler.handlers.size() > 0);

       server_pid = fork();

       if (server_pid < 0) {
           perror("TCPServer fork failed, retrying...");
           sleep(1); // Delay before retrying
           server_pid = fork(); // Retry once
           if (server_pid < 0) {
               perror("TCPServer fork failed, unable to start server");
               return -1;
           }
       }

       if (server_pid == 0) {
           run_server();
           exit(EXIT_SUCCESS);
       }

       return server_pid;
   }
   ```

   - **Explanation**: Adding a retry mechanism and better error handling prevents immediate failure in case `fork()` fails due to transient issues like system resource limits.

### 3. **Close Socket on Error in `run_server()`**
   - **Original**: In `run_server()`, when an error occurs in `bind()`, `listen()`, or `select()`, the socket descriptor is closed, but only once.
   - **Improvement**: Ensure that the socket is closed in every error path to prevent file descriptor leaks.

   ```cpp
   void TCPServer::run_server() {
       server_sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
       if (server_sock_fd < 0) {
           perror("TCPServer socket");
           exit(EXIT_FAILURE);
       }

       struct sockaddr_in6 server_addr;
       memset(&server_addr, 0, sizeof(server_addr));
       server_addr.sin6_family = AF_INET6;
       server_addr.sin6_port = htons(port_no);
       server_addr.sin6_addr = in6addr_any;

       if (bind(server_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
           perror("TCPServer bind");
           close(server_sock_fd); // ensure closing socket
           exit(EXIT_FAILURE);
       }

       if (listen(server_sock_fd, backlog) < 0) {
           perror("TCPServer listen");
           close(server_sock_fd); // ensure closing socket
           exit(EXIT_FAILURE);
       }
       
       // Similar error handling for select and accept here...
   }
   ```

   - **Explanation**: Properly closing sockets in error cases prevents resource leaks, which could cause the server to run out of file descriptors.

### 4. **Threaded Client Handling**
   - **Original**: The `accept()` call forks a process for each client connection, which is resource-intensive.
   - **Improvement**: Replace `fork()` with a thread-based model to handle client connections.

   ```cpp
   void TCPServer::TCPClientHandler::accept(int server_sock_fd) {
       reap_clients();

       struct sockaddr_in6 client_addr;
       socklen_t client_addr_len = sizeof(client_addr);
       int client_sock_fd = ::accept(server_sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
       if (client_sock_fd < 0) {
           perror("TCPClientHandler accept");
           return;
       }

       if (clients.size() >= max_clients) {
           if (debug_mode) {
               fprintf(stderr, "Max clients reached... dropping connection\n");
           }
           close(client_sock_fd);
           return;
       }

       std::thread([this, client_sock_fd, client_addr]() {
           TCPClient* client = new TCPClient(client_sock_fd, client_addr);
           handler(client, extra_data);
           delete client;
           close(client_sock_fd);
       }).detach();
   }
   ```

   - **Explanation**: Using threads instead of `fork()` reduces overhead by avoiding process creation. Detaching the thread allows concurrent handling of multiple clients without blocking the main loop.

### 5. **Avoid Blocking on `waitpid` for Client Processes**
   - **Original**: The server calls `waitpid()` in `reap_clients()`, potentially blocking the main server process.
   - **Improvement**: Use `WNOHANG` to avoid blocking on `waitpid()`.

   ```cpp
   void TCPServer::TCPClientHandler::reap_clients() {
       if (debug_mode) {
           fprintf(stderr, "Reaping clients\n");
       }

       if (clients.empty()) {
           return;
       }

       int reap_count = 0;
       pid_t finished;
       while (!clients.empty() && (finished = waitpid(-1, nullptr, WNOHANG)) > 0) {
           clients.erase(std::remove(clients.begin(), clients.end(), finished), clients.end());
           reap_count++;
       }

       if (debug_mode) {
           fprintf(stderr, "Reaped %d clients\n", reap_count);
       }
   }
   ```

   - **Explanation**: This ensures that `reap_clients()` doesn't block if no child processes have finished, allowing the server to continue accepting new connections.

---

### Summary of Improvements:
1. **Graceful shutdown with retries** for more controlled termination.
2. **Retry on `fork()` failure** for better robustness.
3. **Improved socket handling** to prevent resource leaks.
4. **Switched to threads** for lightweight client handling, reducing the overhead of forking processes.
5. **Non-blocking wait on `waitpid()`** for better performance in handling child processes.

These changes improve the server’s efficiency, robustness, and resource management, making it more scalable and reliable.