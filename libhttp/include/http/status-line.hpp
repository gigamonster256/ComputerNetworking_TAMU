#ifndef __HTTP_STATUS_LINE_HPP__
#define __HTTP_STATUS_LINE_HPP__

#include <string>

#include "http/typedef.hpp"

namespace http {

// Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

enum class StatusCodeEnum {
  OK,
  CREATED,
  ACCEPTED,
  NO_CONTENT,
  MOVED_PERMANENTLY,
  MOVED_TEMPORARILY,
  NOT_MODIFIED,
  BAD_REQUEST,
  UNAUTHORIZED,
  FORBIDDEN,
  NOT_FOUND,
  INTERNAL_SERVER_ERROR,
  NOT_IMPLEMENTED,
  BAD_GATEWAY,
  SERVICE_UNAVAILABLE,
  EXTENSION,
};

class StatusCode {
  StatusCodeEnum code;
  std::optional<int> extension_code;

 public:
  StatusCode() = default;
  StatusCode(const std::string& code);

  std::string to_string() const;

 private:
  StatusCodeEnum get_status_code(const std::string& code);
};

class StatusLine {
  HTTPVersion version;
  StatusCode code;
  std::optional<std::string> reason;

 public:
  StatusLine() = default;
  StatusLine(const std::string& status_line);

  std::string to_string() const;

  const HTTPVersion& get_version() const { return version; }
  const StatusCode& get_code() const { return code; }
  const std::optional<std::string>& get_reason() const { return reason; }
};
}  // namespace http

#endif  // __HTTP_STATUS_LINE_HPP__