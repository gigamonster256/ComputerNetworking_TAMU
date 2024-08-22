#ifndef _TCP_SERVER_HPP_
#define _TCP_SERVER_HPP_

#include "common.hpp"
#include "tcp_client.hpp"

typedef void (*ClientHandlerFunction)(TCPClient*);
typedef void (*TimeoutFunction)();

class TCPServer {
 public:
  class TCPClientHandler {
   public:
    enum handle_mode { RoundRobin, Random };

   private:
    int max_handlers;
    int num_handlers;
    ClientHandlerFunction* handlers;
    int current_handler;
    handle_mode mode;

   public:
    TCPClientHandler();
    TCPClientHandler(const TCPClientHandler&) = delete;
    TCPClientHandler(TCPClientHandler&&) = delete;
    TCPClientHandler& operator=(const TCPClientHandler&) = delete;
    ~TCPClientHandler();
    void add_handler(ClientHandlerFunction handler);
    void set_mode(handle_mode mode);
    friend class TCPServer;

   private:
    pid_t handle(TCPClient* client);
  };

 private:
  int port_no;
  ip_version version;
  int server_sock_fd;
  TCPClientHandler client_handler;
  int timeout;
  int max_timeouts;
  int timeout_counter;
  TimeoutFunction timeout_handler;
  int backlog;
  static bool debug_mode;
  pid_t* child_pids;
  int num_children;
  int max_children;
  pid_t server_pid;
  bool sock_opened;
  static bool should_exit;

 public:
  // bind to port on all interfaces
  TCPServer();
  TCPServer(const TCPServer&) = delete;
  TCPServer(TCPServer&&) = delete;
  TCPServer& operator=(const TCPServer&) = delete;
  ~TCPServer();

  TCPServer& set_port(int port_no);
  TCPServer& add_handler(ClientHandlerFunction handler);
  TCPServer& set_handler_mode(TCPClientHandler::handle_mode mode);
  TCPServer& set_timeout(int seconds);
  TCPServer& set_max_timeouts(int seconds);
  TCPServer& set_timeout_handler(TimeoutFunction handler);
  TCPServer& set_backlog(int size);
  TCPServer& debug(bool mode);
  pid_t start();
  void exec();
  void stop();

 private:
  void start_server();
  void modification_after_start_error(const char* method);
  void add_child(pid_t pid);
  void reap_children();
  void join_children();
  bool started() { return server_pid != -1; }
};


#endif