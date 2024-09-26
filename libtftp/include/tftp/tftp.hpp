#ifndef _TFTP_HPP_
#define _TFTP_HPP_

#include <stdint.h>

#include <ostream>

#define TFTP_PORT 69

#define NETASCII_MODE "netascii"
#define OCTET_MODE "octet"
// mail mode is obsolete and should not be used or implemented

#define MAX_MODE_LEN 8  // max length of mode string (netascii)

#define MAX_FILENAME_LEN 128       // implementation defined
#define MAX_ERROR_MESSAGE_LEN 128  // implementation defined

#define MAX_DATA_LEN 512  // spec defined

#define MAX_PACKET_LEN \
  (4 + (MAX_DATA_LEN))  // data packet is biggest, 2 bytes for opcode, 2 bytes
                        // for block number, 512 bytes for data

namespace tftp {

class Packet {
 public:
  enum class Mode {
    NETASCII,
    OCTET,
  };
  static Mode mode_from_string(const char* mode_str);
  static const char* mode_to_string(Mode mode);

  enum class Opcode : uint16_t {
    RRQ = 1,    // Read Request (RRQ)
    WRQ = 2,    // Write Request (WRQ)
    DATA = 3,   // Data (DATA)
    ACK = 4,    // Acknowledgment (ACK)
    ERROR = 5,  // Error (ERROR)
  };
  static_assert(sizeof(Opcode) == 2, "Opcode must be 2 bytes");
  static const char* opcode_to_string(Opcode opcode);

  enum class ErrorCode : uint16_t {
    NOT_DEFINED = 0,          // Not defined, see error message (if any).
    FILE_NOT_FOUND = 1,       // File not found.
    ACCESS_VIOLATION = 2,     // Access violation.
    DISK_FULL = 3,            // Disk full or allocation exceeded.
    ILLEGAL_OPERATION = 4,    // Illegal TFTP operation.
    UNKNOWN_TRANSFER_ID = 5,  // Unknown transfer ID.
    FILE_ALREADY_EXISTS = 6,  // File already exists.
    NO_SUCH_USER = 7,         // No such user.
  };
  static_assert(sizeof(ErrorCode) == 2, "ErrorCode must be 2 bytes");
  static const char* error_code_to_string(ErrorCode error_code);

  typedef uint16_t block_num;

 private:
  struct RQ {
    char payload[MAX_FILENAME_LEN + MAX_MODE_LEN + 2];  // 2 bytes as null terms
    const char* filename() const;
    const char* mode() const;
    size_t size() const;
  };
  static_assert(sizeof(RQ) == 138, "RQ struct must be 138 bytes");

  struct DATA {
    uint16_t block;
    char data[MAX_DATA_LEN];
    size_t size() const;
  };
  static_assert(sizeof(DATA) == 514, "DATA struct must be 514 bytes");

  struct ACK {
    block_num block;
  };
  static_assert(sizeof(ACK) == 2, "ACK struct must be 2 bytes");

  struct ERROR {
    ErrorCode error_code;
    char error_message[MAX_ERROR_MESSAGE_LEN + 1];  // 1 byte for null term
    size_t size() const;
  };
  // ERROR is 2 byte aligned due to alignment of ErrorCode
  // therefore total size is +1 for padding
  static_assert(sizeof(ERROR) == 132, "ERROR struct must be 132 bytes");

 public:
  Opcode opcode;
  union {
    RQ rq;
    DATA data;
    ACK ack;
    ERROR error;
  } payload;
  size_t size() const;
  friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
};
static_assert(sizeof(Packet) == MAX_PACKET_LEN,
              "Packet struct must be 516 bytes");

}  // namespace tftp

#endif