#ifndef _UDP_SERVER_HPP_
#define _UDP_SERVER_HPP_

#include <vector>

#include "udp/client.hpp"

namespace udp {

typedef void* client_data_ptr_t;

typedef void (*ClientHandlerFunction)(Client*, const char*, size_t,
                                      client_data_ptr_t);
typedef void (*TimeoutFunction)();

class Server {
 public:
  class ClientHandler {
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
    size_t initial_packet_buffer_size;
    client_data_ptr_t extra_data;

    ClientHandler()
        : current_handler(0),
          clients(),
          max_clients(5),
          handlers(),
          mode(RoundRobin),
          debug_mode(false),
          initial_packet_buffer_size(1024),
          extra_data(nullptr) {}
    ~ClientHandler();
    void set_max_clients(unsigned int max) { max_clients = max; }
    void add_handler(ClientHandlerFunction handler) {
      handlers.push_back(handler);
    }
    void set_mode(handle_mode mode) { this->mode = mode; }
    void debug(bool mode) { debug_mode = mode; }
    void set_initial_packet_buffer_size(size_t size) {
      initial_packet_buffer_size = size;
    }
    void set_extra_data(client_data_ptr_t data) { extra_data = data; }

    void accept(int server_sock_fd);
    void reap_clients();
    void join_clients();
    void terminate_clients();
    void kill_clients();

    friend class Server;
  };

 private:
  // operational data
  int server_sock_fd;
  ClientHandler client_handler;
  pid_t server_pid;
  unsigned int timeout_count;

  // configuration data
  char server_ip_addr[INET6_ADDRSTRLEN];
  unsigned int port_no;
  unsigned int timeout;
  unsigned int max_timeouts;
  bool debug_mode;
  TimeoutFunction timeout_handler;

 public:
  Server()
      : server_sock_fd(-1),
        client_handler(),
        server_pid(-1),
        timeout_count(0),
        server_ip_addr(),
        port_no(0),
        timeout(1),
        max_timeouts(0),
        debug_mode(false),
        timeout_handler(nullptr) {}
  ~Server();

  // server configuration
  Server& set_port(unsigned int port_no);
  Server& set_ip_addr(const char* ip_addr);
  Server& set_timeout(unsigned int seconds);
  Server& set_max_timeouts(unsigned int seconds);
  Server& debug(bool mode);
  Server& set_timeout_handler(TimeoutFunction handler);

  // client handler configuration
  Server& add_handler(ClientHandlerFunction handler);
  Server& set_handler_mode(ClientHandler::handle_mode mode);
  Server& set_max_clients(unsigned int max_clients);
  Server& add_handler_extra_data(void* data);
  Server& set_initial_packet_buffer_size(size_t size);

  // server operation
  pid_t start();
  void exec();
  void stop(bool force = false);

 private:
  void run_server();
};

}  // namespace udp

#endif