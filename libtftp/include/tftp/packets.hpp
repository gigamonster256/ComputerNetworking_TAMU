#ifndef _TFTP_MESSAGES_HPP_
#define _TFTP_MESSAGES_HPP_

#include <stddef.h>

#include "tftp/tftp.hpp"

namespace tftp {

const Packet RRQ(const char* filename, const char* mode);

const Packet WRQ(const char* filename, const char* mode);

const Packet DATA(block_num block, const char* data, size_t length);

const Packet ACK(block_num block);

const Packet ERROR(ErrorCode error_code);
const Packet ERROR(ErrorCode error_code, const char* error_message);

const Packet ACK_from(const Packet& packet);

}  // namespace tftp

#endif