#ifndef _HTTP_CLIENT_HPP_
#define _HTTP_CLIENT_HPP_

#include <string>

namespace http {

class Client {
 public:
  void get(std::string host, std::string path);
};

}  // namespace http

#endif  // _HTTP_CLIENT_HPP_