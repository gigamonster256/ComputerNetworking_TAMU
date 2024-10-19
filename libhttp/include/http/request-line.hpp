#ifndef __HTTP_REQUEST_LINE_HPP__
#define __HTTP_REQUEST_LINE_HPP__

#include <string>

#include "http/typedef.hpp"
#include "http/uri.hpp"

namespace http {

// Request-Line = Method SP Request-URI SP HTTP-Version CRLF

class RequestLine {
  Method method;
  URI uri;
  HTTPVersion version;

 public:
  RequestLine() = default;
  RequestLine(const std::string& request_line);

  std::string to_string() const;
  MethodType get_method_type() const { return method.get_type(); }
  std::string get_uri() const { return uri.to_string(); }

  static const RequestLine GET(const std::string& uri);
};

}  // namespace http

#endif  // __HTTP_REQUEST_LINE_HPP__