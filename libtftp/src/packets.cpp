#include "tftp/packets.hpp"

#include <string.h>

#include "tftp/error.hpp"

namespace tftp {

using ErrorCode = Packet::ErrorCode;
using block_num = Packet::block_num;

const Packet RRQ(const char* filename, const char* mode) {
  Packet packet;
  packet.opcode = Packet::Opcode::RRQ;
  size_t filename_len = strnlen(filename, MAX_FILENAME_LEN);
  strncpy(packet.payload.rq.payload, filename, filename_len);
  packet.payload.rq.payload[filename_len] = '\0';
  size_t mode_len = strnlen(mode, MAX_MODE_LEN);
  strncpy(packet.payload.rq.payload + filename_len + 1, mode, mode_len);
  packet.payload.rq.payload[filename_len + mode_len + 1] = '\0';
  return packet;
}

const Packet WRQ(const char* filename, const char* mode) {
  Packet packet;
  packet.opcode = Packet::Opcode::WRQ;
  size_t filename_len = strnlen(filename, MAX_FILENAME_LEN);
  strncpy(packet.payload.rq.payload, filename, filename_len);
  packet.payload.rq.payload[filename_len] = '\0';
  size_t mode_len = strnlen(mode, MAX_MODE_LEN);
  strncpy(packet.payload.rq.payload + filename_len + 1, mode, mode_len);
  packet.payload.rq.payload[filename_len + 1 + mode_len] = '\0';
  return packet;
}

const Packet DATA(block_num block, const char* data, size_t length) {
  Packet packet;
  packet.opcode = Packet::Opcode::DATA;
  packet.payload.data.block = block;
  memcpy(packet.payload.data.data, data, length);
  if (length < MAX_DATA_LEN) {
    packet.payload.data.data[length] = '\0';
  }
  return packet;
}

const Packet ACK(block_num block) {
  Packet packet;
  packet.opcode = Packet::Opcode::ACK;
  packet.payload.ack = {block};
  return packet;
}

const Packet ERROR(ErrorCode error_code, const char* error_message) {
  Packet packet;
  packet.opcode = Packet::Opcode::ERROR;
  packet.payload.error.error_code = error_code;
  strncpy(packet.payload.error.error_message, error_message,
          MAX_ERROR_MESSAGE_LEN);
  return packet;
}

const Packet ACK_from(const Packet& packet) {
  if (packet.opcode != Packet::Opcode::DATA) {
    throw TFTPError("ACK can only be created from DATA packets");
  }
  return ACK(packet.payload.data.block);
}

}  // namespace tftp