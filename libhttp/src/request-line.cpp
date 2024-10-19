#include "http/request-line.hpp"

#include <iostream>
#include <optional>

#include "http/constants.hpp"
#include "http/typedef.hpp"

namespace http {

Method::Method(const std::string& method) : type(get_method_type(method)) {
  if (type == MethodType::EXTENSION) {
    extension_method = method;
  }
}

MethodType Method::get_method_type(const std::string& method) {
  if (method == "GET") return MethodType::GET;
  if (method == "HEAD") return MethodType::HEAD;
  if (method == "POST") return MethodType::POST;
  return MethodType::EXTENSION;
}

std::string Method::to_string() const {
  switch (type) {
    case MethodType::GET:
      return "GET";
    case MethodType::HEAD:
      return "HEAD";
    case MethodType::POST:
      return "POST";
    default:
      return extension_method.value();
  }
}

const Method Method::GET = Method("GET");
const Method Method::HEAD = Method("HEAD");
const Method Method::POST = Method("POST");
const Method Method::EXTENSION(const std::string& extension) {
  return Method(extension);
}

HTTPVersion::HTTPVersion(const std::string& version) {
  size_t slash = version.find('/');
  size_t dot = version.find('.');
  major = std::stoi(version.substr(slash + 1, dot - slash - 1));
  minor = std::stoi(version.substr(dot + 1));
}

const HTTPVersion HTTPVersion::HTTP_1_0 = HTTPVersion("HTTP/1.0");

std::string HTTPVersion::to_string() const {
  return "HTTP/" + std::to_string(major) + "." + std::to_string(minor);
}

RequestLine::RequestLine(const std::string& request_line) {
  size_t first_space = request_line.find(' ');
  size_t second_space = request_line.find(' ', first_space + 1);
  method = Method(request_line.substr(0, first_space));
  uri =
      URI(request_line.substr(first_space + 1, second_space - first_space - 1));
  version = HTTPVersion(request_line.substr(second_space + 1));
}

std::string RequestLine::to_string() const {
  return method.to_string() + " " + uri.to_string() + " " + version.to_string();
}

const RequestLine RequestLine::GET(const std::string& uri) {
  return RequestLine("GET " + uri + " " + HTTPVersion::HTTP_1_0.to_string());
}

}  // namespace http