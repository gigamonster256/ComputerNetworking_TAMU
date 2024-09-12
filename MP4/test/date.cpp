#include <ctime>
#include <iostream>
#include <string>

#include "http/date.hpp"
#include "http/error.hpp"

int main() {
  time_t t = time(0);
  struct tm* now_tm = gmtime(&t);
  char buffer[128];

  strftime(buffer, sizeof(buffer), RFC_1123_DATE_FORMAT, now_tm);
  std::string now_1123 = std::string(buffer);

  strftime(buffer, sizeof(buffer), RFC_850_DATE_FORMAT, now_tm);
  std::string now_850 = std::string(buffer);

  strftime(buffer, sizeof(buffer), ANSI_C_DATE_FORMAT, now_tm);
  std::string now_ansi = std::string(buffer);

  std::cout << "RFC 1123: " << now_1123 << std::endl;
  std::cout << "RFC 850: " << now_850 << std::endl;
  std::cout << "ANSI C: " << now_ansi << std::endl;

  http::HTTPDate date(now_1123);
  std::cout << "HTTPDate: " << date << std::endl;
  date = http::HTTPDate(now_850);
  std::cout << "HTTPDate: " << date << std::endl;
  date = http::HTTPDate(now_ansi);
  std::cout << "HTTPDate: " << date << std::endl;

  try {
    // missing comma after wkday
    http::HTTPDate date("Wed 09 Jun 2021 10:18:14 GMT");
    exit(EXIT_FAILURE);
  } catch (http::DateParseError& e) {
    std::cerr << "DateParseError: " << e.what() << std::endl;
  }

  try {
    // garbage date
    http::HTTPDate date("garbage");
    exit(EXIT_FAILURE);
  } catch (http::DateParseError& e) {
    std::cerr << "DateParseError: " << e.what() << std::endl;
  }
}