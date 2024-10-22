#pragma once

#include <ostream>
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
  std::string reason_phrase() const;
  StatusCodeEnum get_code() const { return code; }
  int get_extension_code() const { return extension_code.value(); }

  friend std::ostream& operator<<(std::ostream& os, const StatusCode& code);

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
