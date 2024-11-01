#include <unistd.h>

#include <iostream>
#include <map>
#include <memory>

#include "http/client.hpp"
#include "http/constants.hpp"
#include "http/message.hpp"
#include "http/status-line.hpp"
#include "tcp/client.hpp"
#include "tcp/server.hpp"

using namespace http;
using namespace tcp;

void usage(const char* progname) {
  std::cerr << "Usage: " << progname << " <server_port>" << std::endl;
  exit(EXIT_FAILURE);
}

typedef std::unique_ptr<http::Message> (*RouteResponse)();

// 1. a cache hit returns the saved data to the requester;
static int test_1_count = 0;
std::unique_ptr<http::Message> test_1() {
  auto current_count = test_1_count++;
  std::string body = "Test 1: " + std::to_string(current_count);
  auto response =
      std::make_unique<http::Message>(StatusCode(StatusCodeEnum::OK));

  response->add_header("Content-Type", "text/plain");
  // inefficient, but whatever
  response->add_header("Date", Date(time(nullptr)).to_string());
  response->add_header("Content-Length", std::to_string(body.size()));
  // expires in 60 seconds
  response->add_header("Expires", Date(time(nullptr) + 60).to_string());

  response->set_body(body);
  return response;
}

// 4. a stale Expires header in the cache is accessed, the cache entry is
// replaced with a fresh copy, and the fresh data is delivered to the requester
static int test_4_count = 0;
std::unique_ptr<http::Message> test_4() {
  auto current_count = test_4_count++;
  std::string body = "Test 4: " + std::to_string(current_count);
  auto response =
      std::make_unique<http::Message>(StatusCode(StatusCodeEnum::OK));

  response->add_header("Content-Type", "text/plain");
  response->add_header("Date", Date(time(nullptr)).to_string());
  // expires 10 seconds ago... therefore stale
  response->add_header("Expires", Date(time(nullptr) - 10).to_string());
  response->add_header("Content-Length", std::to_string(body.size()));

  response->set_body(body);
  return response;
}

// 5. a stale entry in the cache without an Expires header is
// determined  based  on  the  last  Web  server  access  time  and  last
// modification time, the stale cache entry is replaced with fresh data, and
// the fresh data is delivered to the requester

// when subsequest requests are made for this resource, the main function
// should return a 304 if the If-Modified-Since header is present
static int test_5_count = 0;
std::unique_ptr<http::Message> test_5() {
  auto current_count = test_5_count++;
  std::string body = "Test 5: " + std::to_string(current_count);
  auto response =
      std::make_unique<http::Message>(StatusCode(StatusCodeEnum::OK));

  response->add_header("Content-Type", "text/plain");
  response->add_header("Date", Date(time(nullptr)).to_string());
  // last modified 10 seconds ago... therefore stale
  response->add_header("Last-Modified", Date(time(nullptr) - 10).to_string());
  response->add_header("Content-Length", std::to_string(body.size()));

  response->set_body(body);
  return response;
}

// 6.  cache entry without an
// Expires  header  that  has  been  previously  accessed  from  the  Web
// server in the last 24 hours and was last modified more than one month
// ago  is  returned  to  the  requester
static int test_6_count = 0;
std::unique_ptr<http::Message> test_6() {
  auto current_count = test_6_count++;
  std::string body = "Test 6: " + std::to_string(current_count);
  auto response =
      std::make_unique<http::Message>(StatusCode(StatusCodeEnum::OK));

  response->add_header("Content-Type", "text/plain");
  response->add_header("Date", Date(time(nullptr)).to_string());
  // last modified 1 month ago... therefore stale
  response->add_header("Last-Modified",
                       Date(time(nullptr) - 60 * 60 * 24 * 30).to_string());
  response->add_header("Content-Length", std::to_string(body.size()));

  response->set_body(body);
  return response;
}

// 7. three  clients  can  simultaneously access the proxy server and get the
// correct data
static int test_7_count = 0;
std::unique_ptr<http::Message> test_7() {
  auto current_count = test_7_count++;
  std::string body = "Test 7: " + std::to_string(current_count);
  auto response =
      std::make_unique<http::Message>(StatusCode(StatusCodeEnum::OK));

  response->add_header("Content-Type", "text/plain");
  response->add_header("Date", Date(time(nullptr)).to_string());
  response->add_header("Content-Length", std::to_string(body.size()));

  response->set_body(body);

  // sleep for 10 seconds to simulate a slow response
  sleep(10);
  return response;
}

// map route strings to handler functions
std::map<std::string, RouteResponse> routes = {
    {"/test1", test_1}, {"/test4", test_4}, {"/test5", test_5},
    {"/test6", test_6}, {"/test7", test_7},
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage(argv[0]);
  }

  int port = atoi(argv[1]);

  tcp::Server server;

  server.set_ip_addr("::")
      .set_port(port)
      .use_threads()
      .add_handler([](tcp::Client* client, void*) {
        std::string request_str;
        char buffer[1024];
        while (true) {
          size_t bytes_read = client->read(buffer, sizeof(buffer));
          if (bytes_read == 0) {
            break;
          }
          request_str.append(buffer, bytes_read);
          if (request_str.size() >= 4 &&
              request_str.substr(request_str.size() - 4) ==
                  std::string(CRLF) + std::string(CRLF)) {
            break;
          }
        }
        http::Message request(request_str);
        auto uri = request.get_uri();
        std::cerr << "Request for " << uri << std::endl;

        auto route = routes.find(uri);
        if (route != routes.end()) {
          std::cout << "200" << std::endl;
          // check for If-Modified-Since header
          if (auto if_modified_since_header =
                  request.get_header("If-Modified-Since")) {
            std::cerr << "If-Modified-Since header found" << std::endl;

            // all other trests return 200 but test5 returns 304
            if (uri == "/test5") {
              std::cout << "304" << std::endl;
              auto response = std::make_unique<http::Message>(
                  StatusCode(StatusCodeEnum::NOT_MODIFIED));
              auto response_str = response->to_string();
              client->writen((void*)response_str.c_str(), response_str.size());
              return;
            } else {
              goto normal_response;
            }
          } else {
          normal_response:
            auto response = route->second();
            auto response_str = response->to_string();
            client->writen((void*)response_str.c_str(), response_str.size());
          }
        }
        // 404 if the route is not found
        else {
          std::cout << "404" << std::endl;
          auto response = std::make_unique<http::Message>(
              StatusCode(StatusCodeEnum::NOT_FOUND));
          auto response_str = response->to_string();
          client->writen((void*)response_str.c_str(), response_str.size());
        }
      })
      .exec();
}