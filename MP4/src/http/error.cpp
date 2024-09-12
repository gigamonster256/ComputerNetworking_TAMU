#include "error.hpp"

namespace http {

const char* HTTPError::what() const noexcept { return "HTTP error"; }

HeaderParseError::HeaderParseError(const std::string& header)
    : header(header) {}
const char* HeaderParseError::what() const noexcept { return header.c_str(); }

DateParseError::DateParseError(const std::string& date) : date(date) {}
const char* DateParseError::what() const noexcept { return date.c_str(); }

}  // namespace http