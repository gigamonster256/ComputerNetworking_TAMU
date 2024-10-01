#ifndef __UDP_ERROR_HPP__
#define __UDP_ERROR_HPP__

#include <stdexcept>

namespace udp {

class ConfigurationError : public std::runtime_error {
 public:
  ConfigurationError(const std::string& what_arg)
      : std::runtime_error(what_arg) {}
  ConfigurationError(const char* what_arg) : std::runtime_error(what_arg) {}
};

}  // namespace tcp

#endif