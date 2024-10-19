#include "http/status-line.hpp"

#include "http/constants.hpp"
#include "http/typedef.hpp"

namespace http {

StatusCode::StatusCode(const std::string& code) : code(get_status_code(code)) {
  if (this->code == StatusCodeEnum::EXTENSION) {
    extension_code = std::stoi(code);
  }
}

StatusCodeEnum StatusCode::get_status_code(const std::string& code) {
  if (code == "200") return StatusCodeEnum::OK;
  if (code == "201") return StatusCodeEnum::CREATED;
  if (code == "202") return StatusCodeEnum::ACCEPTED;
  if (code == "204") return StatusCodeEnum::NO_CONTENT;
  if (code == "301") return StatusCodeEnum::MOVED_PERMANENTLY;
  if (code == "302") return StatusCodeEnum::MOVED_TEMPORARILY;
  if (code == "304") return StatusCodeEnum::NOT_MODIFIED;
  if (code == "400") return StatusCodeEnum::BAD_REQUEST;
  if (code == "401") return StatusCodeEnum::UNAUTHORIZED;
  if (code == "403") return StatusCodeEnum::FORBIDDEN;
  if (code == "404") return StatusCodeEnum::NOT_FOUND;
  if (code == "500") return StatusCodeEnum::INTERNAL_SERVER_ERROR;
  if (code == "501") return StatusCodeEnum::NOT_IMPLEMENTED;
  if (code == "502") return StatusCodeEnum::BAD_GATEWAY;
  if (code == "503") return StatusCodeEnum::SERVICE_UNAVAILABLE;
  return StatusCodeEnum::EXTENSION;
}

std::string StatusCode::to_string() const {
  switch (code) {
    case StatusCodeEnum::OK:
      return "200 OK";
    case StatusCodeEnum::CREATED:
      return "201 Created";
    case StatusCodeEnum::ACCEPTED:
      return "202 Accepted";
    case StatusCodeEnum::NO_CONTENT:
      return "204 No Content";
    case StatusCodeEnum::MOVED_PERMANENTLY:
      return "301 Moved Permanently";
    case StatusCodeEnum::MOVED_TEMPORARILY:
      return "302 Moved Temporarily";
    case StatusCodeEnum::NOT_MODIFIED:
      return "304 Not Modified";
    case StatusCodeEnum::BAD_REQUEST:
      return "400 Bad Request";
    case StatusCodeEnum::UNAUTHORIZED:
      return "401 Unauthorized";
    case StatusCodeEnum::FORBIDDEN:
      return "403 Forbidden";
    case StatusCodeEnum::NOT_FOUND:
      return "404 Not Found";
    case StatusCodeEnum::INTERNAL_SERVER_ERROR:
      return "500 Internal Server Error";
    case StatusCodeEnum::NOT_IMPLEMENTED:
      return "501 Not Implemented";
    case StatusCodeEnum::BAD_GATEWAY:
      return "502 Bad Gateway";
    case StatusCodeEnum::SERVICE_UNAVAILABLE:
      return "503 Service Unavailable";
    case StatusCodeEnum::EXTENSION:
      return std::to_string(extension_code.value());
  }
}

StatusLine::StatusLine(const std::string& status_line) {
  auto first_space = status_line.find(' ');
  version = HTTPVersion(status_line.substr(0, first_space));
  auto second_space = status_line.find(' ', first_space + 1);
  code = StatusCode(
      status_line.substr(first_space + 1, second_space - first_space - 1));
  auto crlf = status_line.find(CRLF, second_space + 1);
  if (crlf - second_space > 1)
    reason = status_line.substr(second_space + 1, crlf - second_space - 1);
}

std::string StatusLine::to_string() const {
  std::string status_line = version.to_string();
  status_line += ' ';
  status_line += code.to_string();
  status_line += ' ';
  if (reason) {
    status_line += *reason;
  }
  return status_line;
}

}  // namespace http