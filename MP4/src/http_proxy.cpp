#include <unistd.h>

#include <iostream>

#include "http/client.hpp"
#include "http/constants.hpp"
#include "http/message.hpp"
#include "tcp/client.hpp"
#include "tcp/server.hpp"

using namespace http;
using namespace tcp;

void usage(const char* progname) {
  std::cerr << "Usage: " << progname << " <ip to bind> <port>" << std::endl;
  exit(EXIT_FAILURE);
}

typedef time_t ExpirationTime;
typedef time_t LastUsedTime;
typedef std::tuple<LastUsedTime, ExpirationTime, std::unique_ptr<Message>>
    CacheEntry;
typedef std::unordered_map<size_t, CacheEntry> Cache;

Cache cache;
std::mutex cache_mutex;

std::pair<std::string, std::string> get_host_and_path_from_uri(
    const std::string& uri) {
  // strip the http://<host> part
  size_t pos = uri.find("://");
  std::string host;
  if (pos != std::string::npos) {
    host = uri.substr(pos + 3);
  } else {
    host = uri;
  }

  // strip the path part
  pos = host.find("/");
  std::string path;
  if (pos != std::string::npos) {
    path = host.substr(pos);
    host = host.substr(0, pos);
  } else {
    path = "/";
  }

  return std::make_pair(host, path);
}

void print_welcome_message(char* ip, int port) {
  std::cerr << "Starting proxy on " << ip << ":" << port << std::endl;
  std::cerr << "Press Ctrl+C to stop the proxy" << std::endl;
  std::cerr << "Send SIGUSR1 to print cache summary" << std::endl;
  std::cerr << "PID: " << getpid() << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage(argv[0]);
  }

  char* ip = argv[1];
  int port = atoi(argv[2]);

  print_welcome_message(ip, port);

  auto extra_data = std::make_tuple(&cache, &cache_mutex);

  // on user1 interrupt, print a summary of the cache
  signal(SIGUSR1, [](int) {
    std::cerr << "Cache summary:" << std::endl;
    for (auto& [hash, entry] : cache) {
      std::cerr << "Hash: " << hash << std::endl;
      std::cerr << "Last used: " << std::get<0>(entry) << std::endl;
      std::cerr << "Expiration: " << std::get<1>(entry) << std::endl;
      std::cerr << "Response: " << *std::get<2>(entry) << std::endl;
    }
  });

  tcp::Server server;
  server.set_ip_addr(ip)
      .set_port(port)
      .use_threads()  // use threads to handle multiple clients instead of
                      // forking -- new feature in MP4
      .add_handler_extra_data(&extra_data)
      .add_handler([](tcp::Client* client, void* extra_data) {
        auto [cache, cache_mutex] =
            *static_cast<std::tuple<Cache*, std::mutex*>*>(extra_data);
        // read in the http request from the client (get request ends with
        // 2CRLFs)
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

        auto now = time(nullptr);

        // get the requested uri
        std::string uri = request.get_uri();
        std::cerr << "Request for " << uri << std::endl;

        // get the host and path
        auto [host, path] = get_host_and_path_from_uri(uri);
        auto additional_headers = HeaderList{};

        // check if the uri is in the cache
        size_t hash = std::hash<std::string>{}(uri);
        auto it = cache->find(hash);
        // if an entry exists in the cache and it is not expired
        if (it != cache->end()) {
          CacheEntry& entry = it->second;
          auto expiration_time = std::get<1>(entry);
          // if the entry is not expired
          if (now < expiration_time) {
            std::cerr << "Cache hit for " << uri << std::endl;
            std::cerr << "Re-send the response" << std::endl;
            std::cerr << "Expires in " << expiration_time - now << " seconds"
                      << std::endl;
            // update the last used time
            std::get<0>(entry) = now;
            std::string response_str = std::get<2>(entry)->to_string();
            client->writen((void*)response_str.c_str(), response_str.size());
            return;
          }
          // stale cache entry -- make a conditional get
          std::cerr << "Stale cache entry for " << uri << std::endl;
          std::cerr << "Sending if modified since header" << std::endl;
          // build a request with If-Modified-Since header
          auto last_used_time_tm = *gmtime(&expiration_time);
          char if_modified_since[128];
          strftime(if_modified_since, sizeof(if_modified_since),
                   "If-Modified-Since: %a, %d %b %Y %H:%M:%S GMT",
                   &last_used_time_tm);
          additional_headers.push_back(Header::parse_header(if_modified_since));
        } else {
          std::cerr << "Cache miss for " << uri << std::endl;
        }

        // create a client to the server
        http::Client http_client(host);
        // get the response from the server
        auto response = http_client.get(path, std::move(additional_headers));

        auto status_code = response->get_status_code();
        std::cerr << "Response status: " << status_code.to_string()
                  << std::endl;

        // check if the response is a 304 Not Modified
        if (status_code.get_code() == StatusCodeEnum::NOT_MODIFIED) {
          std::cerr << "Serving from cache" << std::endl;
          auto& entry = cache->at(hash);
          // update the last used time
          std::get<0>(entry) = now;
          // write the response back to the client
          auto response_str = std::get<2>(entry)->to_string();
          client->writen((void*)response_str.c_str(), response_str.size());
          return;
        }

        auto expiration_time = now;  // default to not caching

        // check for Expires header
        auto expires_header = response->get_header("Expires");
        if (expires_header) {
          std::cerr << "Expires header found" << std::endl;
          std::cerr << *expires_header << std::endl;
          // cast the header to a ExpiresHeader
          auto expires_header_cast =
              dynamic_cast<const ExpiresHeader*>(expires_header);
          expiration_time = expires_header_cast->get_date().get_time();
          std::cerr << "Expires in " << expiration_time - now << " seconds"
                    << std::endl;
        } else {
          auto last_modified_header = response->get_header("Last-Modified");
          if (last_modified_header) {
            std::cerr << "Last-Modified header found" << std::endl;
            std::cerr << *last_modified_header << std::endl;
            // cast the header to a LastModifiedHeader
            auto last_modified_header_cast =
                dynamic_cast<const LastModifiedHeader*>(last_modified_header);
            expiration_time = last_modified_header_cast->get_date().get_time();
            std::cerr << "Expires in " << expiration_time - now << " seconds"
                      << std::endl;
          } else {
            auto date_header = response->get_header("Date");
            if (date_header) {
              std::cerr << "Date header found" << std::endl;
              std::cerr << *date_header << std::endl;
              // cast the header to a DateHeader
              auto date_header_cast =
                  dynamic_cast<const DateHeader*>(date_header);
              expiration_time = date_header_cast->get_date().get_time();
              std::cerr << "Expires in " << expiration_time - now << " seconds"
                        << std::endl;
            } else {
              std::cerr << "No expiration information found" << std::endl;
            }
          }
        }

        // write the response back to the client
        std::string response_str = response->to_string();
        client->writen((void*)response_str.c_str(), response_str.size());

        cache->emplace(hash,
                       CacheEntry{now, expiration_time, std::move(response)});

        // if cache has 11 entries, remove the oldest one
        if (cache->size() > 10) {
          auto oldest = cache->begin();
          for (auto it = cache->begin(); it != cache->end(); ++it) {
            if (std::get<0>(it->second) < std::get<0>(oldest->second)) {
              oldest = it;
            }
          }
          cache->erase(oldest);
        }
      })
      // .debug(true)
      .exec();
  // .start();

  // // periodically clean the cache
  // while (true) {
  //   sleep(10);
  //   std::cerr << "Cleaning cache" << std::endl;
  //   int count = 0;
  //   // clean up expired cache entries
  //   auto now = time(nullptr);
  //   for (auto it = cache.begin(); it != cache.end();) {
  //     CacheEntry& entry = it->second;
  //     ExpirationTime expiration_time = std::get<1>(entry);
  //     if (now >= expiration_time) {
  //       it = cache.erase(it);
  //       ++count;
  //     } else {
  //       ++it;
  //     }
  //   }
  //   std::cerr << "Cleaned " << count << " expired entries" << std::endl;
  // }
}