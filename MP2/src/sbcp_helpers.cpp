#include "sbcp_helpers.hpp"

#include <iostream>

namespace sbcp {

char helper_scratch_print_buf[SBCP_MAX_PAYLOAD_LENGTH];

void print_attr_type(const attribute_type_t type) {
  switch (type) {
    case attribute_type_t::USERNAME:
      std::cout << "USERNAME";
      break;
    case attribute_type_t::MESSAGE:
      std::cout << "MESSAGE";
      break;
    case attribute_type_t::REASON:
      std::cout << "REASON";
      break;
    case attribute_type_t::CLIENT_COUNT:
      std::cout << "CLIENT_COUNT";
      break;
    default:
      std::cout << "UNKNOWN";
      break;
  }
}

void print_attr(const attribute_t& attr) {
  std::cout << "Attribute: ";
  print_attr_type(attr.get_type());
  std::cout << " (length: " << attr.get_length() << ")";
  std::cout << " - ";
  switch (attr.get_type()) {
    case attribute_type_t::USERNAME:
      std::memcpy(helper_scratch_print_buf, attr.get_username(),
                  attr.get_length());
      helper_scratch_print_buf[attr.get_length()] = '\0';
      std::cout << helper_scratch_print_buf;
      break;
    case attribute_type_t::MESSAGE:
      std::memcpy(helper_scratch_print_buf, attr.get_message(),
                  attr.get_length());
      helper_scratch_print_buf[attr.get_length()] = '\0';
      std::cout << helper_scratch_print_buf;
      break;
    case attribute_type_t::REASON:
      std::memcpy(helper_scratch_print_buf, attr.get_reason(),
                  attr.get_length());
      helper_scratch_print_buf[attr.get_length()] = '\0';
      std::cout << helper_scratch_print_buf;
      break;
    case attribute_type_t::CLIENT_COUNT:
      std::cout << attr.get_client_count();
      break;
    default:
      break;
  }
}

void print_message_type(const message_type_t type) {
  switch (type) {
    case message_type_t::JOIN:
      std::cout << "JOIN";
      break;
    case message_type_t::SEND:
      std::cout << "SEND";
      break;
    case message_type_t::FWD:
      std::cout << "FWD";
      break;
    case message_type_t::ACK:
      std::cout << "ACK";
      break;
    case message_type_t::NAK:
      std::cout << "NAK";
      break;
    case message_type_t::ONLINE:
      std::cout << "ONLINE";
      break;
    case message_type_t::OFFLINE:
      std::cout << "OFFLINE";
      break;
    case message_type_t::IDLE:
      std::cout << "IDLE";
      break;
    default:
      std::cout << "UNKNOWN";
      break;
  }
}

void print_message(const message_t& msg) {
  std::cout << "Message: ";
  print_message_type(msg.get_type());
  std::cout << " (length: " << msg.get_length() << ")";
  int i = 0;
  for (const auto& attr : msg) {
    (void)attr;
    std::cout << std::endl << "  ";
    print_attr(msg[i]);
    i++;
  }
}

}  // namespace sbcp