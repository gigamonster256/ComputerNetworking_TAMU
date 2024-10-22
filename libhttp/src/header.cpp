#include "http/header.hpp"

#include <memory>
#include <string>

#include "http/constants.hpp"
#include "http/date.hpp"
#include "http/error.hpp"

namespace http {

enum class Header::HeaderType {
  Date,
  Expires,
  LastModified,
  Unknown,
};
Header::HeaderType Header::get_header_type(const std::string& name) {
  if (name == "Date") return HeaderType::Date;
  if (name == "Expires") return HeaderType::Expires;
  if (name == "Last-Modified") return HeaderType::LastModified;
  return HeaderType::Unknown;
}

Header::Header(const std::string& name, const std::string& value)
    : name(name), value(value) {}

std::string Header::to_string() const { return name + ": " + value; }

GeneralHeader::GeneralHeader(const std::string& name, const std::string& value)
    : Header(name, value) {}

EntityHeader::EntityHeader(const std::string& name, const std::string& value)
    : Header(name, value) {}

DateHeader::DateHeader(const std::string& value)
    : GeneralHeader("Date", value), date(value) {}
std::string DateHeader::to_string() const {
  return name + ": " + date.to_string();
}

ExpiresHeader::ExpiresHeader(const Date& date)
    : EntityHeader("Expires", date.to_string()), date(date) {}

ExpiresHeader::ExpiresHeader(const std::string& value)
    : EntityHeader("Expires", value), date(value) {}
std::string ExpiresHeader::to_string() const {
  return name + ": " + date.to_string();
}

LastModifiedHeader::LastModifiedHeader(const std::string& value)
    : EntityHeader("Last-Modified", value), date(value) {}
std::string LastModifiedHeader::to_string() const {
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
    case HeaderType::Expires: {
      // https://datatracker.ietf.org/doc/html/rfc1945#section-10.7
      // Applications are encouraged to be tolerant of bad or
      // misinformed implementations of the Expires header. A value of zero
      // (0) or an invalid date format should be considered equivalent to
      // an "expires immediately."
      try {
        return std::make_unique<ExpiresHeader>(value);
      } catch (DateParseError&) {
        return std::make_unique<ExpiresHeader>(Date(time(nullptr)));
      }
    }
    case HeaderType::LastModified:
      return std::make_unique<LastModifiedHeader>(value);
    default:
      return std::make_unique<ExtensionHeader>(name, value);
  }
}

}  // namespace http