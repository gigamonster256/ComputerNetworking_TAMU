#ifndef _TFTP_ERROR_HPP_
#define _TFTP_ERROR_HPP_

#include <stdexcept>
#include <string>

#include "tftp/tftp.hpp"

namespace tftp {

class TFTPError : public std::runtime_error {
 public:
  TFTPError(const char* what);
  TFTPError(const std::string& what);
};

class InvalidModeError : public TFTPError {
 public:
  InvalidModeError(const char* mode);
  InvalidModeError(const std::string& mode);
  InvalidModeError(const Mode mode);
};

}  // namespace tftp

#endif  // _TFTP_ERROR_HPP_