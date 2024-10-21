#pragma once

#include <string>
#include <string_view>

#define HTTP_PORT 80

namespace http {
constexpr std::string_view CRLF = "\r\n";
}  // namespace http