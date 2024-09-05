#ifndef _UDP_SERVER_HPP_
#define _UDP_SERVER_HPP_

#include <mutex>
#include <vector>

#include "udp_common.hpp"
#include "udp_client.hpp"

typedef void* client_data_ptr_t;

typedef void (*ClientHandlerFunction)(UDPClient*, client_data_ptr_t);
typedef void (*TimeoutFunction)();

class UDPServer {
 public:
  class UDPClientHandler {
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

    UDPClientHandler()
        : current_handler(0),
          clients(),
          max_clients(5),
          handlers(),
          mode(RoundRobin),
          debug_mode(false),
          extra_data(nullptr) {}
    ~UDPClientHandler();
    void set_max_clients(unsigned int max) { max_clients = max; }
    void add_handler(ClientHandlerFunction handler) {
      handlers.push_back(handler);
    }
    void set_mode(handle_mode mode) { this->mode = mode; }
    void debug(bool mode) { debug_mode = mode; }
    void set_extra_data(client_data_ptr_t data) { extra_data = data; }

    void accept(int server_sock_fd);
    void reap_clients();
    void join_clients();
    void terminate_clients();
    void kill_clients();

    friend class UDPServer;
  };

 private:
  // operational data
  int server_sock_fd;
  UDPClientHandler client_handler;
  pid_t server_pid;
  unsigned int timeout_count;

  // configuration data
  unsigned int port_no;
  unsigned int timeout;
  unsigned int max_timeouts;
  bool debug_mode;
  TimeoutFunction timeout_handler;

 public:
  UDPServer()
      : server_sock_fd(-1),
        client_handler(),
        server_pid(-1),
        timeout_count(0),
        port_no(-1),
        timeout(1),
        max_timeouts(0),
        debug_mode(false),
        timeout_handler(nullptr) {}
  ~UDPServer();

  // server configuration
  UDPServer& set_port(unsigned int port_no);
  UDPServer& set_timeout(unsigned int seconds);
  UDPServer& set_max_timeouts(unsigned int seconds);
  UDPServer& debug(bool mode);
  UDPServer& set_timeout_handler(TimeoutFunction handler);

  // client handler configuration
  UDPServer& add_handler(ClientHandlerFunction handler);
  UDPServer& set_handler_mode(UDPClientHandler::handle_mode mode);
  UDPServer& set_max_clients(unsigned int max_clients);
  UDPServer& add_handler_extra_data(void* data);

  // server operation
  pid_t start();
  void exec();
  void stop(bool force = false);

 private:
  void run_server();
};

#endif