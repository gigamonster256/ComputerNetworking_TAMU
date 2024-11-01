#include "http/message.hpp"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

#include "http/constants.hpp"
#include "http/header.hpp"
#include "http/typedef.hpp"

namespace http {

Message::Message(const std::string& message) {
  size_t pos = 0;
  size_t end = message.find(CRLF, pos);
  if (end == std::string::npos) {
    std::cerr << "Oops! " << message << std::endl;
    std::cerr << message << std::endl;
    throw std::runtime_error("Invalid message");
  }
  std::string first_line_str = message.substr(pos, end - pos);
  if (first_line_str.find("HTTP") == 0) {
    first_line = StatusLine(first_line_str);
  } else {
    first_line = RequestLine(first_line_str);
  }
  pos = end + CRLF.size();
  end = message.find(CRLF, pos);
  while (end != pos) {
    std::string header_str = message.substr(pos, end - pos);
    headers.push_back(Header::parse_header(header_str));
    pos = end + CRLF.size();
    end = message.find(CRLF, pos);
  }
  pos += CRLF.size();
  if (pos < message.size()) {
    body = message.substr(pos);
  }
}

Message& Message::add_header(std::unique_ptr<Header> header) {
  headers.push_back(std::move(header));
  return *this;
}

Message& Message::add_header(const std::string& name,
                             const std::string& value) {
  return add_header(Header::parse_header(name, value));
}

const Header* Message::get_header(const std::string& name) {
  for (const auto& header : headers) {
    if (header->get_name() == name) {
      return header.get();
    }
  }
  return nullptr;
}

Message& Message::set_body(const std::string& body) {
  this->body = body;
  return *this;
}

const std::string Message::to_string() const {
  std::string message =
      std::visit([](const auto& first_line) { return first_line.to_string(); },
                 first_line);
  message += CRLF;
  for (const auto& header : headers) {
    message += header->to_string();
    message += CRLF;
  }
  message += CRLF;
  if (body) {
    message += *body;
  }
  return message;
}

const StatusCode& Message::get_status_code() const {
  assert(std::holds_alternative<StatusLine>(first_line));
  return std::get<StatusLine>(first_line).get_code();
}

MethodType Message::get_method() const {
  assert(std::holds_alternative<RequestLine>(first_line));
  return std::get<RequestLine>(first_line).get_method_type();
}

std::string Message::get_uri() const {
  assert(std::holds_alternative<RequestLine>(first_line));
  return std::get<RequestLine>(first_line).get_uri();
}

Message Message::GET(const std::string& path) {
  return Message(RequestLine::GET(path));
}

std::ostream& operator<<(std::ostream& os, const Message& message) {
  os << message.to_string();
  return os;
}

}  // namespace http