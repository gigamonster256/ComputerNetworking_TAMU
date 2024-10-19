#ifndef __HTTP_MESSAGE_HPP__
#define __HTTP_MESSAGE_HPP__

#include <memory>
#include <optional>
#include <string>

#include "http/header.hpp"
#include "http/request-line.hpp"
#include "http/status-line.hpp"
#include "http/typedef.hpp"

namespace http {

class Message {
 protected:
  std::variant<RequestLine, StatusLine> first_line;
  std::unique_ptr<HeaderList> headers;
  std::optional<std::string> body;

 public:
  Message(const RequestLine& request_line)
      : first_line(request_line), headers(std::make_unique<HeaderList>()) {}
  Message(RequestLine&& request_line)
      : first_line(std::move(request_line)),
        headers(std::make_unique<HeaderList>()) {}
  Message(const std::string& message);
  virtual ~Message() = default;

  Message& set_headers(std::unique_ptr<HeaderList> headers);
  Message& add_header(std::unique_ptr<Header> header);
  Message& add_header(const std::string& name, const std::string& value);
  Message& set_body(const std::string& body);

  const std::string to_string() const;
  const HeaderList& get_headers() const { return *headers; }

  static Message GET(const std::string& path);

  friend std::ostream& operator<<(std::ostream& os, const Message& message);
};

}  // namespace http

#endif  // __HTTP_MESSAGE_HPP__