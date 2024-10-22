#include "http/error.hpp"

namespace http {

char error_buffer[1024];

HeaderParseError::HeaderParseError(const std::string& header)
    : HTTPError(), header(header) {}

const char* HeaderParseError::what() const noexcept {
  snprintf(error_buffer, sizeof(error_buffer), "Header parse error: %s",
           header.c_str());
  return error_buffer;
}

DateParseError::DateParseError(const std::string& date)
    : HTTPError(), date(date) {}

const char* DateParseError::what() const noexcept {
  snprintf(error_buffer, sizeof(error_buffer), "Date parse error: %s",
           date.c_str());
  return error_buffer;
}

}  // namespace http