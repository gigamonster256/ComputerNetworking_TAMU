#include "http/header.hpp"

#include <iostream>

#include "http/error.hpp"

using namespace http;

int main() {
  auto header = Header::parse_header("Date: Wed, 09 Jun 2021 10:18:14 GMT");
  if (!header) {
    std::cerr << "Failed to parse header" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << header->to_string() << std::endl;

  // test invalid header
  try {
    header = Header::parse_header("Invalid header");
    exit(EXIT_FAILURE);
  } catch (HeaderParseError& e) {
    std::cerr << "HeaderParseError: " << e.what() << std::endl;
  }

  // invalid date
  try {
    header = Header::parse_header("Date: Invalid date");
    exit(EXIT_FAILURE);
  } catch (DateParseError& e) {
    std::cerr << "DateParseError: " << e.what() << std::endl;
  }

  // test date gets turned from RFC 850 to RFC 1123
  header = Header::parse_header("Date: Wednesday, 09-Jun-21 10:18:14 GMT");
  if (!header) {
    std::cerr << "Failed to parse header" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << header << std::endl;

  return 0;
}