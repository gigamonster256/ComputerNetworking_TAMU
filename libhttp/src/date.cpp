#include "http/date.hpp"

#include <ctime>
#include <string>

#include "http/error.hpp"

namespace http {

Date::Date(const std::string& date) {
  struct tm tm;
  if (strptime(date.c_str(), RFC_1123_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm) - timezone + (daylight ? 3600 : 0);
    return;
  }

  if (strptime(date.c_str(), RFC_850_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm) - timezone + (daylight ? 3600 : 0);
    return;
  }

  if (strptime(date.c_str(), ANSI_C_DATE_FORMAT, &tm) != nullptr) {
    this->date = mktime(&tm) - timezone + (daylight ? 3600 : 0);
    return;
  }

  throw DateParseError(date);
}

bool Date::operator<(const Date& other) const {
  return this->date < other.date;
}

bool Date::operator>(const Date& other) const {
  return this->date > other.date;
}

bool Date::operator<=(const Date& other) const {
  return this->date <= other.date;
}

bool Date::operator>=(const Date& other) const {
  return this->date >= other.date;
}

bool Date::operator==(const Date& other) const {
  return this->date == other.date;
}

bool Date::operator!=(const Date& other) const {
  return this->date != other.date;
}

std::string Date::to_string() const {
  struct tm* tm = gmtime(&date);
  char buffer[128];
  strftime(buffer, sizeof(buffer), RFC_1123_DATE_FORMAT, tm);
  return std::string(buffer);
}

std::ostream& operator<<(std::ostream& os, const Date& date) {
  os << date.to_string();
  return os;
}

}  // namespace http