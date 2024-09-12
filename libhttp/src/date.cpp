#include "date.hpp"

#include <ctime>
#include <string>

#include "error.hpp"

namespace http {

HTTPDate::HTTPDate(const std::string& date) {
  struct tm tm;
  if (strptime(date.c_str(), RFC_1123_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm);
    return;
  }

  if (strptime(date.c_str(), RFC_850_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm);
    return;
  }

  if (strptime(date.c_str(), ANSI_C_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm);
    return;
  }

  throw DateParseError(date);
}

bool HTTPDate::operator<(const HTTPDate& other) const {
  return this->date < other.date;
}

bool HTTPDate::operator>(const HTTPDate& other) const {
  return this->date > other.date;
}

bool HTTPDate::operator<=(const HTTPDate& other) const {
  return this->date <= other.date;
}

bool HTTPDate::operator>=(const HTTPDate& other) const {
  return this->date >= other.date;
}

bool HTTPDate::operator==(const HTTPDate& other) const {
  return this->date == other.date;
}

bool HTTPDate::operator!=(const HTTPDate& other) const {
  return this->date != other.date;
}

std::string HTTPDate::to_string() const {
  struct tm* tm = gmtime(&date);
  char buffer[128];
  strftime(buffer, sizeof(buffer), RFC_1123_DATE_FORMAT, tm);
  return std::string(buffer);
}

std::ostream& operator<<(std::ostream& os, const HTTPDate& date) {
  os << date.to_string();
  return os;
}

}  // namespace http