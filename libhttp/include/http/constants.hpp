#ifndef _HTTP_CONSTANTS_HPP_
#define _HTTP_CONSTANTS_HPP_

#include <string>
#include <string_view>

#define HTTP_PORT 80

namespace http {
constexpr std::string_view CRLF = "\r\n";
}  // namespace http

#endif  // _HTTP_CONSTANTS_HPP_