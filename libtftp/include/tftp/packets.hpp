#ifndef _TFTP_MESSAGES_HPP_
#define _TFTP_MESSAGES_HPP_

#include <stddef.h>

#include "tftp/tftp.hpp"

namespace tftp {

const Packet RRQ(const char* filename, const char* mode);

const Packet WRQ(const char* filename, const char* mode);

const Packet DATA(Packet::block_num block, const char* data, size_t length);

const Packet ACK(Packet::block_num block);

const Packet ERROR(Packet::ErrorCode error_code, const char* error_message);

}  // namespace tftp

#endif