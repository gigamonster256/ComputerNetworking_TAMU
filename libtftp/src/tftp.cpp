#include "tftp/tftp.hpp"

#include <string.h>

#include <string>

#include "tftp/error.hpp"

namespace tftp {

using Mode = Packet::Mode;

Mode Packet::mode_from_string(const char* mode_str) {
  if (strcmp(mode_str, NETASCII_MODE) == 0) {
    return Mode::NETASCII;
  } else if (strcmp(mode_str, OCTET_MODE) == 0) {
    return Mode::OCTET;
  }
  throw InvalidModeError(mode_str);
}

const char* Packet::mode_to_string(Mode mode) {
  switch (mode) {
    case Mode::NETASCII:
      return NETASCII_MODE;
    case Mode::OCTET:
      return OCTET_MODE;
  }
  return "UNKNOWN";
}

const char* Packet::opcode_to_string(Opcode opcode) {
  switch (opcode) {
    case Opcode::RRQ:
      return "RRQ";
    case Opcode::WRQ:
      return "WRQ";
    case Opcode::DATA:
      return "DATA";
    case Opcode::ACK:
      return "ACK";
    case Opcode::ERROR:
      return "ERROR";
  }
  return "UNKNOWN";
}

const char* Packet::error_code_to_string(ErrorCode error_code) {
  switch (error_code) {
    case ErrorCode::NOT_DEFINED:
      return "Not defined, see error message (if any)";
    case ErrorCode::FILE_NOT_FOUND:
      return "File not found";
    case ErrorCode::ACCESS_VIOLATION:
      return "Access violation";
    case ErrorCode::DISK_FULL:
      return "Disk full or allocation exceeded";
    case ErrorCode::ILLEGAL_OPERATION:
      return "Illegal TFTP operation";
    case ErrorCode::UNKNOWN_TRANSFER_ID:
      return "Unknown transfer ID";
    case ErrorCode::FILE_ALREADY_EXISTS:
      return "File already exists";
    case ErrorCode::NO_SUCH_USER:
      return "No such user";
  }
  return "UNKNOWN";
}

const char* Packet::RQ::filename() const { return payload; }

const char* Packet::RQ::mode() const {
  const size_t filename_len = strnlen(payload, MAX_FILENAME_LEN) + 1;
  return payload + filename_len;
}

size_t Packet::RQ::size() const {
  const size_t filename_len = strnlen(payload, MAX_FILENAME_LEN) + 1;
  const size_t mode_len = strnlen(payload + filename_len, MAX_MODE_LEN) + 1;
  return filename_len + mode_len;
}

size_t Packet::DATA::size() const {
  const size_t data_len = strnlen(data, MAX_DATA_LEN);
  return sizeof(block) + data_len;
}

size_t Packet::ERROR::size() const {
  const size_t error_message_len = strnlen(error_message, MAX_ERROR_MESSAGE_LEN) + 1;
  return sizeof(error_code) + error_message_len;
}

size_t Packet::size() const {
  switch (opcode) {
    case Opcode::RRQ:
    case Opcode::WRQ:
      return sizeof(opcode) + payload.rq.size();
    case Opcode::DATA:
      return sizeof(opcode) + payload.data.size();
    case Opcode::ACK:
      return sizeof(opcode) + sizeof(ACK);
    case Opcode::ERROR:
      return sizeof(opcode) + payload.error.size();
  }
  throw TFTPError("Unknown opcode" +
                  std::to_string(static_cast<uint16_t>(opcode)));
}

std::ostream& operator<<(std::ostream& os, const Packet& packet) {
  os << "Opcode: " << Packet::opcode_to_string(packet.opcode) << std::endl;
  switch (packet.opcode) {
    case Packet::Opcode::RRQ:
    case Packet::Opcode::WRQ:
      os << "  Filename: " << packet.payload.rq.filename() << std::endl;
      os << "  Mode: " << packet.payload.rq.mode();
      break;
    case Packet::Opcode::DATA:
      os << "  Block: " << packet.payload.data.block << std::endl;
      os << "  Data: " << packet.payload.data.data;
      break;
    case Packet::Opcode::ACK:
      os << "  Block: " << packet.payload.ack.block;
      break;
    case Packet::Opcode::ERROR:
      os << "  Error code: " << Packet::error_code_to_string(packet.payload.error.error_code)
         << std::endl;
      os << "  Error message: " << packet.payload.error.error_message;
      break;
  }
  return os;
}

}  // namespace tftp