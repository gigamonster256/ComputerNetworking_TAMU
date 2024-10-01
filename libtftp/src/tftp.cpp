#include "tftp/tftp.hpp"

#include <string.h>

#include <string>

#include "tftp/error.hpp"

namespace tftp {

const char* Mode::to_string() const {
  switch (mode) {
    case Value::NETASCII:
      return NETASCII_MODE;
    case Value::OCTET:
      return OCTET_MODE;
  }
  return "UNKNOWN";
}

Mode Mode::from_string(const char* mode_str) {
  if (strcmp(mode_str, NETASCII_MODE) == 0) {
    return Mode(Value::NETASCII);
  } else if (strcmp(mode_str, OCTET_MODE) == 0) {
    return Mode(Value::OCTET);
  }
  throw TFTPError("Unknown mode: " + std::string(mode_str));
}

const char* Opcode::to_string() const {
  switch (opcode) {
    case Value::RRQ:
      return "Read Request (RRQ)";
    case Value::WRQ:
      return "Write Request (WRQ)";
    case Value::DATA:
      return "Data (DATA)";
    case Value::ACK:
      return "Acknowledgment (ACK)";
    case Value::ERROR:
      return "Error (ERROR)";
  }
  return "UNKNOWN";
}

const char* ErrorCode::to_string() const {
  switch (error_code) {
    case Value::NOT_DEFINED:
      return "Not defined, see error message (if any)";
    case Value::FILE_NOT_FOUND:
      return "File not found";
    case Value::ACCESS_VIOLATION:
      return "Access violation";
    case Value::DISK_FULL:
      return "Disk full or allocation exceeded";
    case Value::ILLEGAL_OPERATION:
      return "Illegal TFTP operation";
    case Value::UNKNOWN_TRANSFER_ID:
      return "Unknown transfer ID";
    case Value::FILE_ALREADY_EXISTS:
      return "File already exists";
    case Value::NO_SUCH_USER:
      return "No such user";
  }
  return "UNKNOWN";
}

Packet::RQ::RQ(const char* filename, const char* mode) {
  size_t filename_len = strnlen(filename, TFTP_MAX_FILENAME_LEN);
  strncpy(payload, filename, filename_len);
  payload[filename_len] = '\0';
  size_t mode_len = strnlen(mode, TFTP_MAX_MODE_LEN);
  strncpy(payload + filename_len + 1, mode, mode_len);
  payload[filename_len + mode_len + 1] = '\0';
}

const char* Packet::RQ::filename() const { return payload; }

const char* Packet::RQ::mode() const {
  const size_t filename_len = strnlen(payload, TFTP_MAX_FILENAME_LEN) + 1;
  return payload + filename_len;
}

size_t Packet::RQ::size() const {
  const size_t filename_len = strnlen(payload, TFTP_MAX_FILENAME_LEN) + 1;
  const size_t mode_len =
      strnlen(payload + filename_len, TFTP_MAX_MODE_LEN) + 1;
  return filename_len + mode_len;
}

Packet::DATA::DATA(block_num block, const char* data, size_t length) {
  this->block = htons(block);
  const size_t data_len = std::min(length, size_t{TFTP_MAX_DATA_LEN});
  memcpy(this->data, data, data_len);
}

block_num Packet::DATA::get_block() const { return ntohs(block); }

// warning: may be non-null terminated
const char* Packet::DATA::get_data() const { return data; }

Packet::ACK::ACK(block_num block) { this->block = htons(block); }

block_num Packet::ACK::get_block() const { return ntohs(block); }

size_t Packet::ACK::size() const { return sizeof(block); }

Packet::ERROR::ERROR(ErrorCode error_code) : error_code(error_code) {
  strncpy(error_message, "", TFTP_MAX_ERROR_MESSAGE_LEN);
  error_message[TFTP_MAX_ERROR_MESSAGE_LEN] = '\0';
}

Packet::ERROR::ERROR(ErrorCode error_code, const char* error_message) {
  this->error_code = error_code;
  strncpy(this->error_message, error_message, TFTP_MAX_ERROR_MESSAGE_LEN);
  this->error_message[TFTP_MAX_ERROR_MESSAGE_LEN] = '\0';
}

ErrorCode Packet::ERROR::get_error_code() const { return error_code; }

const char* Packet::ERROR::get_error_message() const { return error_message; }

size_t Packet::ERROR::size() const {
  const size_t error_message_len =
      strnlen(error_message, TFTP_MAX_ERROR_MESSAGE_LEN) + 1;
  return sizeof(error_code) + error_message_len;
}

size_t Packet::size() const {
  switch (opcode) {
    case Opcode::RRQ:
    case Opcode::WRQ:
      return sizeof(opcode) + payload.rq.size();
    case Opcode::DATA:
      throw TFTPError("DATA packets should be constructed with a length");
    case Opcode::ACK:
      return sizeof(opcode) + payload.ack.size();
    case Opcode::ERROR:
      return sizeof(opcode) + payload.error.size();
  }
  throw TFTPError("Unknown opcode" +
                  std::to_string(static_cast<uint16_t>(opcode)));
}

Packet::Packet(Opcode opcode, const RQ& rq) : opcode(opcode) {
  payload.rq = rq;
}

Packet::Packet(const DATA& data) {
  opcode = Opcode::DATA;
  payload.data = data;
}

Packet::Packet(const ACK& ack) {
  opcode = Opcode::ACK;
  payload.ack = ack;
}

Packet::Packet(const ERROR& error) {
  opcode = Opcode::ERROR;
  payload.error = error;
}

std::ostream& operator<<(std::ostream& os, const Packet& packet) {
  os << "Opcode: " << packet.opcode.to_string() << std::endl;
  switch (packet.opcode) {
    case Opcode::RRQ:
    case Opcode::WRQ:
      os << "  Filename: " << packet.payload.rq.filename() << std::endl;
      os << "  Mode: " << packet.payload.rq.mode();
      break;
    case Opcode::DATA:
      os << "  Block: " << packet.payload.data.get_block();
      break;
    case Opcode::ACK:
      os << "  Block: " << packet.payload.ack.get_block();
      break;
    case Opcode::ERROR:
      os << "  Error code: "
         << packet.payload.error.get_error_code().to_string() << std::endl;
      os << "  Error message: " << packet.payload.error.get_error_message();
      break;
  }
  return os;
}

}  // namespace tftp