#ifndef _HTTP_DATE_HPP_
#define _HTTP_DATE_HPP_

#define RFC_1123_DATE_FORMAT "%a, %d %b %Y %H:%M:%S GMT"
#define RFC_850_DATE_FORMAT "%A, %d-%b-%y %H:%M:%S GMT"
#define ANSI_C_DATE_FORMAT "%a %b %e %H:%M:%S %Y"

#include <ctime>
#include <string>
#include <iostream>

namespace http {

class HTTPDate {
 private:
  std::time_t date;

 public:
  HTTPDate(const std::string& date);

  bool operator<(const HTTPDate& other) const;
  bool operator>(const HTTPDate& other) const;
  bool operator<=(const HTTPDate& other) const;
  bool operator>=(const HTTPDate& other) const;
  bool operator==(const HTTPDate& other) const;
  bool operator!=(const HTTPDate& other) const;

  std::string to_string() const;
  friend std::ostream& operator<<(std::ostream& os, const HTTPDate& date);
};

}  // namespace http

#endif  // _HTTP_DATE_HPP_