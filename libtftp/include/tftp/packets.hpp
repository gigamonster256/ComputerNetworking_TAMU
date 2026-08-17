#pragma once

#include <stddef.h>

#include "tftp/tftp.hpp"

namespace tftp {

Packet RRQ(const char* filename, const char* mode);

Packet WRQ(const char* filename, const char* mode);

Packet DATA(block_num block, const char* data, size_t length);

Packet ACK(block_num block);

Packet ERROR(ErrorCode error_code);
Packet ERROR(ErrorCode error_code, const char* error_message);

Packet ACK_from(const Packet& packet);

}  // namespace tftp
