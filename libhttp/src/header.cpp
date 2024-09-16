#include "http/header.hpp"

#include <string>

#include "http/constants.hpp"
#include "http/date.hpp"
#include "http/error.hpp"

namespace http {

enum class Header::HeaderType {
  Date,
  Unknown,
};
Header::HeaderType Header::get_header_type(const std::string& name) {
  if (name == "Date") return HeaderType::Date;
  return HeaderType::Unknown;
}

Header::Header(const std::string& name, const std::string& value)
    : name(name), value(value) {}

std::string Header::to_string() { return name + ": " + value + CRLF; }

GeneralHeader::GeneralHeader(const std::string& name, const std::string& value)
    : Header(name, value) {}

DateHeader::DateHeader(const std::string& value)
    : GeneralHeader("Date", value), date(value) {}
std::string DateHeader::to_string() {
  return name + ": " + date.to_string() + CRLF;
}

Header* Header::parse_header(const std::string& header) {
  std::string name;
  std::string value;
  size_t colon_pos = header.find(':');
  if (colon_pos == std::string::npos) {
    throw HeaderParseError(header);
  }
  name = header.substr(0, colon_pos);
  // ignore leading whitespace
  size_t value_start = header.find_first_not_of(" \t", colon_pos + 1);
  if (value_start == std::string::npos) {
    throw HeaderParseError(header);
  }
  value = header.substr(value_start);
  switch (get_header_type(name)) {
    case HeaderType::Date:
      return new DateHeader(value);
    default:
      throw HeaderParseError(header);
  }
}

}  // namespace http