#ifndef _HTTP_ERROR_HPP_
#define _HTTP_ERROR_HPP_

#include <exception>
#include <string>

namespace http {

class HTTPError : public std::exception {
 public:
  virtual const char* what() const noexcept override;
};

class HeaderParseError : public HTTPError {
 private:
  std::string header;

 public:
  HeaderParseError(const std::string& header);
  const char* what() const noexcept override;
};

class DateParseError : public HTTPError {
 private:
  std::string date;

 public:
  DateParseError(const std::string& date);
  const char* what() const noexcept override;
};

}  // namespace http

#endif  // _HTTP_ERROR_HPP_