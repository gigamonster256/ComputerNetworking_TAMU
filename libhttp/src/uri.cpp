#include "http/uri.hpp"

namespace http {

URI::URI(const std::string& uri) {
  size_t octothorpe = uri.find_last_of('#');
  if (octothorpe != std::string::npos) {
    this->fragment = uri.substr(octothorpe + 1);
  }
  size_t pos = uri.find(':');
  if (pos != std::string::npos) {
    std::string scheme = uri.substr(0, pos);
    std::string path = uri.substr(pos + 1, octothorpe - pos - 1);
    this->uri = AbsoluteURI{scheme, path};
  } else {
    std::string path = uri.substr(0, octothorpe);
    this->uri = RelativeURI{path};
  }
}

std::string URI::to_string() const {
  std::string result;
  if (std::holds_alternative<AbsoluteURI>(uri)) {
    AbsoluteURI absolute_uri = std::get<AbsoluteURI>(uri);
    result += absolute_uri.scheme;
    result += ":";
    result += absolute_uri.path;
  } else {
    result += std::get<RelativeURI>(uri);
  }
  if (fragment.has_value()) {
    result += "#";
    result += fragment.value();
  }
  return result;
}

const std::string& URI::get_path() const {
  if (std::holds_alternative<AbsoluteURI>(uri)) {
    return std::get<AbsoluteURI>(uri).path;
  } else {
    return std::get<RelativeURI>(uri);
  }
}

}  // namespace http