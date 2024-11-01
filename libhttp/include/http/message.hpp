#pragma once

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
  HeaderList headers;
  std::optional<std::string> body;

 public:
  explicit Message(const RequestLine& request_line) : first_line(request_line) {}
  explicit Message(RequestLine&& request_line) : first_line(std::move(request_line)) {}
  explicit Message(const StatusCode& status_code) : first_line(StatusLine(status_code)) {}
  explicit Message(StatusCode&& status_code) : first_line(StatusLine(std::move(status_code))) {}
  explicit Message(const std::string& message);
  virtual ~Message() = default;

  Message& add_header(std::unique_ptr<Header> header);
  Message& add_header(const std::string& name, const std::string& value);
  const Header* get_header(const std::string& name);
  Message& set_body(const std::string& body);

  const std::string to_string() const;
  const HeaderList& get_headers() const { return headers; }
  const std::optional<std::string>& get_body() const { return body; }
  const StatusCode& get_status_code() const;
  MethodType get_method() const;
  std::string get_uri() const;

  static Message GET(const std::string& path);

  friend std::ostream& operator<<(std::ostream& os, const Message& message);
};

}  // namespace http
