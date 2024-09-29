#include "tftp/packets.hpp"

#include <string.h>

#include "tftp/error.hpp"

namespace tftp {

const Packet RRQ(const char* filename, const char* mode) {
  return Packet(Opcode::RRQ, Packet::RQ(filename, mode));
}

const Packet WRQ(const char* filename, const char* mode) {
  return Packet(Opcode::WRQ, Packet::RQ(filename, mode));
}

const Packet DATA(block_num block, const char* data, size_t length) {
  return Packet(Packet::DATA(block, data, length));
}

const Packet ACK(block_num block) { return Packet(Packet::ACK(block)); }

const Packet ERROR(ErrorCode error_code) {
  return Packet(Packet::ERROR(error_code));
}

const Packet ERROR(ErrorCode error_code, const char* error_message) {
  return Packet(Packet::ERROR(error_code, error_message));
}

const Packet ACK_from(const Packet& packet) {
  if (packet.opcode != Opcode::DATA) {
    throw TFTPError("ACK_from called with non-DATA packet");
  }
  return Packet(Packet::ACK(packet.payload.data.get_block()));
}

}  // namespace tftp