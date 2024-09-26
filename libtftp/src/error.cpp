#include "tftp/error.hpp"

#include <string>

namespace tftp {

TFTPError::TFTPError(const char* what) : std::runtime_error(what) {}
TFTPError::TFTPError(const std::string& what) : std::runtime_error(what) {}

InvalidModeError::InvalidModeError(const char* mode)
    : TFTPError("Invalid mode: " + std::string(mode)) {}
InvalidModeError::InvalidModeError(const std::string& mode) : TFTPError("Invalid mode: " + mode) {}
InvalidModeError::InvalidModeError(const tftp::Packet::Mode mode)
    : TFTPError("Invalid mode: " + std::string(Packet::mode_to_string(mode))) {}

}  // namespace tftp