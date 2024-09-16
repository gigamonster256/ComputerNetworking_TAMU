#ifndef _HTTP_HEADER_HPP_
#define _HTTP_HEADER_HPP_

#include <string>

#include "http/date.hpp"

namespace http {

class Header {
 private:
  enum class HeaderType;
  static HeaderType get_header_type(const std::string &name);

 protected:
  std::string name;
  std::string value;
  Header(const std::string &name, const std::string &value);

 public:
  virtual ~Header() = default;
  virtual std::string to_string();

  static Header *parse_header(const std::string &header);
};

class GeneralHeader : public Header {
 protected:
  GeneralHeader(const std::string &name, const std::string &value);
};

class DateHeader : public GeneralHeader {
 private:
  Date date;

 public:
  DateHeader(const std::string &value);
  std::string to_string() override;
};

}  // namespace http

#endif  // _HTTP_HEADER_HPP_
