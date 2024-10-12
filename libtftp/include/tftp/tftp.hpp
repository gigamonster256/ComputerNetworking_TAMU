#ifndef _TFTP_HPP_
#define _TFTP_HPP_

#include <arpa/inet.h>

#include <cstddef>
#include <ostream>

#define TFTP_PORT 8080

#define NETASCII_MODE "netascii"
#define OCTET_MODE "octet"
// mail mode is obsolete and should not be used or implemented

#define TFTP_MAX_MODE_LEN 8  // max length of mode string (netascii)

#define TFTP_MAX_FILENAME_LEN 128       // implementation defined
#define TFTP_MAX_ERROR_MESSAGE_LEN 128  // implementation defined

#define TFTP_MAX_DATA_LEN 512  // spec defined

#define TFTP_MAX_PACKET_LEN \
  (4 + (TFTP_MAX_DATA_LEN))  // data packet is biggest, 2 bytes for opcode, 2
                             // bytes
                             // for block number, 512 bytes for data

namespace tftp {

class Mode {
 public:
  enum Value {
    NETASCII,
    OCTET,
  };

 private:
  Value mode;

 public:
  Mode() = default;
  constexpr Mode(Value mode) : mode(mode) {}
  constexpr operator Value() const { return mode; }
  explicit operator bool() = delete;
  const char* to_string() const;
  static Mode from_string(const char* mode_str);
};

class Opcode {
 public:
  // for some reason htons is not constexpr on certain systems
  // this hardcoding of values is a workaround and
  // makes this code only work on little endian systems
  enum Value : uint16_t {
    RRQ = 256U,     // Read Request (RRQ)
    WRQ = 512U,     // Write Request (WRQ)
    DATA = 768U,    // Data (DATA)
    ACK = 1024U,    // Acknowledgment (ACK)
    ERROR = 1280U,  // Error (ERROR)
  };

 private:
  Value opcode;

 public:
  Opcode() = default;
  constexpr Opcode(Value opcode) : opcode(opcode) {}
  constexpr operator Value() const { return opcode; }
  explicit operator bool() = delete;
  const char* to_string() const;
};

class ErrorCode {
 public:
  enum Value : uint16_t {
    NOT_DEFINED = 0U,             // Not defined, see error message (if any).
    FILE_NOT_FOUND = 256U,        // File not found.
    ACCESS_VIOLATION = 512U,      // Access violation.
    DISK_FULL = 768U,             // Disk full or allocation exceeded.
    ILLEGAL_OPERATION = 1024U,    // Illegal TFTP operation.
    UNKNOWN_TRANSFER_ID = 1280U,  // Unknown transfer ID.
    FILE_ALREADY_EXISTS = 1536U,  // File already exists.
    NO_SUCH_USER = 1792U,         // No such user.
  };

 private:
  Value error_code;

 public:
  ErrorCode() = default;
  constexpr ErrorCode(Value error_code) : error_code(error_code) {}
  constexpr operator Value() const { return error_code; }
  explicit operator bool() = delete;
  const char* to_string() const;
};

typedef uint16_t block_num;

class Packet {
 public:
  class RQ {
    char payload[TFTP_MAX_FILENAME_LEN + TFTP_MAX_MODE_LEN + 2];

   public:
    RQ(const char* filename, const char* mode);
    const char* filename() const;
    const char* mode() const;
    size_t size() const;
  };
  static_assert(sizeof(RQ) == 138, "RQ struct must be 138 bytes");

  class DATA {
    block_num block;
    char data[TFTP_MAX_DATA_LEN];

   public:
    DATA(block_num block, const char* data, size_t length);
    block_num get_block() const;
    const char* get_data() const;
  };
  static_assert(sizeof(DATA) == 514, "DATA struct must be 514 bytes");

  class ACK {
   private:
    block_num block;

   public:
    ACK(block_num block);
    block_num get_block() const;
    size_t size() const;
  };
  static_assert(sizeof(ACK) == 2, "ACK struct must be 2 bytes");

  class ERROR {
    ErrorCode error_code;
    char error_message[TFTP_MAX_ERROR_MESSAGE_LEN + 1];  // 1 byte for null term
   public:
    ERROR(ErrorCode error_code);
    ERROR(ErrorCode error_code, const char* error_message);
    ErrorCode get_error_code() const;
    const char* get_error_message() const;
    size_t size() const;
  };
  // ERROR is 2 byte aligned due to alignment of ErrorCode
  // therefore total size is +1 for padding
  static_assert(sizeof(ERROR) == 132, "ERROR struct must be 132 bytes");

  Opcode opcode;
  union U {
    RQ rq;
    DATA data;
    ACK ack;
    ERROR error;
    U() {}
  } payload;

 public:
  Packet(Opcode opcode, const RQ& rq);
  Packet(const DATA& data);
  Packet(const ACK& ack);
  Packet(const ERROR& error);
  size_t size() const;
  friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
};
// static_assert(sizeof(Packet) == TFTP_MAX_PACKET_LEN,
//               "Packet struct must be 516 bytes");

}  // namespace tftp

#endif