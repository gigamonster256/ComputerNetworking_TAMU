#ifndef _TCP_SERVER_HPP_
#define _TCP_SERVER_HPP_

#include <mutex>
#include <vector>

#include "common.hpp"
#include "tcp_client.hpp"

typedef void* client_data_ptr_t;

typedef void (*ClientHandlerFunction)(TCPClient*, client_data_ptr_t);
typedef void (*TimeoutFunction)();

class TCPServer {
 public:
  class TCPClientHandler {
   public:
    enum handle_mode { RoundRobin, Random };

   private:
    // operational data
    unsigned int current_handler;
    std::vector<pid_t> clients;

    // configuration data
    unsigned int max_clients;
    std::vector<ClientHandlerFunction> handlers;
    handle_mode mode;
    bool debug_mode;
    client_data_ptr_t extra_data;

    TCPClientHandler()
        : current_handler(0),
          max_clients(5),
          handlers(),
          mode(RoundRobin),
          debug_mode(false),
          extra_data(nullptr) {}
    ~TCPClientHandler();
    void set_max_clients(unsigned int max) { max_clients = max; }
    void add_handler(ClientHandlerFunction handler) {
      handlers.push_back(handler);
    }
    void set_mode(handle_mode mode) { this->mode = mode; }
    void debug(bool mode) { debug_mode = mode; }
    void set_extra_data(client_data_ptr_t data) { extra_data = data; }

    void handle(int client_sock_fd, struct sockaddr_in6);
    void reap_clients();
    void join_clients();
    void terminate_clients();
    void kill_clients();

    friend class TCPServer;
  };

 private:
  // operational data
  int server_sock_fd;
  TCPClientHandler client_handler;
  pid_t server_pid;
  unsigned int timeout_count;

  // configuration data
  unsigned int port_no;
  unsigned int timeout;
  unsigned int max_timeouts;
  unsigned int backlog;
  bool debug_mode;
  TimeoutFunction timeout_handler;

 public:
  TCPServer()
      : server_sock_fd(-1),
        server_pid(-1),
        port_no(-1),
        timeout(1),
        max_timeouts(0),
        backlog(5),
        debug_mode(false),
        timeout_handler(nullptr) {}
  ~TCPServer();

  // server configuration
  TCPServer& set_port(unsigned int port_no);
  TCPServer& set_timeout(unsigned int seconds);
  TCPServer& set_max_timeouts(unsigned int seconds);
  TCPServer& set_backlog(unsigned int size);
  TCPServer& debug(bool mode);
  TCPServer& set_timeout_handler(TimeoutFunction handler);

  // client handler configuration
  TCPServer& add_handler(ClientHandlerFunction handler);
  TCPServer& set_handler_mode(TCPClientHandler::handle_mode mode);
  TCPServer& set_max_clients(unsigned int max_clients);
  TCPServer& add_client_extra_data(void* data);

  // server operation
  pid_t start();
  void exec();
  void stop(bool force = false);

 private:
  void run_server();
};

#endif