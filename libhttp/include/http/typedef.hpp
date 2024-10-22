#pragma once

#include <string>

namespace http {

typedef std::string token;

// Method = "GET" | "HEAD" | "POST" | extension-method

enum class MethodType {
  GET,
  HEAD,
  POST,
  EXTENSION,
};

class Method {
  MethodType type;
  std::optional<token> extension_method;

 public:
  Method() = default;
  Method(const std::string& method);

  std::string to_string() const;
  MethodType get_type() const { return type; }

 private:
  static MethodType get_method_type(const std::string& method);

 public:
  static const Method GET;
  static const Method HEAD;
  static const Method POST;
  static const Method EXTENSION(const std::string& extension);
};

// HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT

class HTTPVersion {
  int major;
  int minor;

 public:
  HTTPVersion() = default;
  HTTPVersion(const std::string& version);

  std::string to_string() const;
  int get_major() const { return major; }
  int get_minor() const { return minor; }

  static const HTTPVersion HTTP_1_0;
};

}  // namespace http
