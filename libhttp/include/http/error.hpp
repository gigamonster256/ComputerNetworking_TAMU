#pragma once

#include <exception>
#include <stdexcept>
#include <string>

namespace http {

class HTTPError : public std::runtime_error {
 public:
  HTTPError() : std::runtime_error("HTTP Error") {}
};

class HeaderParseError : public HTTPError {
 private:
  std::string header;

 public:
  explicit HeaderParseError(const std::string& header);
  const char* what() const noexcept override;
};

class DateParseError : public HTTPError {
 private:
  std::string date;

 public:
  explicit DateParseError(const std::string& date);
  const char* what() const noexcept override;
};

}  // namespace http
