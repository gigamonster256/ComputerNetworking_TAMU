#pragma once

#include <memory>
#include <string>
#include <vector>

#include "http/date.hpp"

namespace http {

class Header;

typedef std::vector<std::unique_ptr<Header>> HeaderList;

class Header {
 private:
  enum class HeaderType;
  static HeaderType get_header_type(const std::string &name);

 protected:
  std::string name;
  std::string value;
  explicit Header(const std::string &name, const std::string &value);

 public:
  virtual ~Header() = default;
  virtual std::string to_string() const;

  const std::string &get_name() const { return name; }
  const std::string &get_value() const { return value; }

  static std::unique_ptr<Header> parse_header(const std::string &header);
  static std::unique_ptr<Header> parse_header(const std::string &name,
                                              const std::string &value);

  friend std::ostream &operator<<(std::ostream &os, const Header &header);
};

class GeneralHeader : public Header {
 protected:
  explicit GeneralHeader(const std::string &name, const std::string &value);
};

class EntityHeader : public Header {
 protected:
  explicit EntityHeader(const std::string &name, const std::string &value);
};

class DateHeader : public GeneralHeader {
 private:
  Date date;

 public:
  explicit DateHeader(const std::string &value);
  std::string to_string() const override;
  const Date &get_date() const { return date; }
};

class ExpiresHeader : public EntityHeader {
 private:
  Date date;

 public:
  explicit ExpiresHeader(const Date &date);
  explicit ExpiresHeader(const std::string &value);
  std::string to_string() const override;
  const Date &get_date() const { return date; }
};

class LastModifiedHeader : public EntityHeader {
 private:
  Date date;

 public:
  explicit LastModifiedHeader(const std::string &value);
  std::string to_string() const override;
  const Date &get_date() const { return date; }
};

class ExtensionHeader : public Header {
 public:
  explicit ExtensionHeader(const std::string &name, const std::string &value);
};

}  // namespace http
