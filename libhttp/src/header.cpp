#include "http/header.hpp"

#include <memory>
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

std::string Header::to_string() const { return name + ": " + value; }

GeneralHeader::GeneralHeader(const std::string& name, const std::string& value)
    : Header(name, value) {}

DateHeader::DateHeader(const std::string& value)
    : GeneralHeader("Date", value), date(value) {}
std::string DateHeader::to_string() const {
  return name + ": " + date.to_string();
}

std::unique_ptr<Header> Header::parse_header(const std::string& header) {
  size_t colon = header.find(':');
  if (colon == std::string::npos) {
    throw HeaderParseError(header);
  }
  return parse_header(header.substr(0, colon), header.substr(colon + 2));
}

std::ostream& operator<<(std::ostream& os, const Header& header) {
  os << header.to_string();
  return os;
}

ExtensionHeader::ExtensionHeader(const std::string& name,
                                 const std::string& value)
    : Header(name, value) {}

std::unique_ptr<Header> Header::parse_header(const std::string& name,
                                             const std::string& value) {
  switch (get_header_type(name)) {
    case HeaderType::Date:
      return std::make_unique<DateHeader>(value);
    default:
      return std::make_unique<ExtensionHeader>(name, value);
  }
}

}  // namespace http