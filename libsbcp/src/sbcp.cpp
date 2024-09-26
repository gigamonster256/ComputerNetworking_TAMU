#include "sbcp/sbcp.hpp"

#include <assert.h>
#include <string.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace sbcp {

Message::Attribute::Attribute(type_t type, const char* value, size_t length)
    : type(type), length(0), payload() {
  switch (type) {
    case type_t::USERNAME:
      set_username(value, length);
      break;
    case type_t::MESSAGE:
      set_message(value, length);
      break;
    case type_t::REASON:
      set_reason(value, length);
      break;
    default:
      throw MessageException("Invalid attribute type");
  }
}

Message::Attribute::Attribute(type_t type, const char* value)
    : type(type), length(0), payload() {
  switch (type) {
    case type_t::USERNAME:
      set_username(value, strlen(value));
      break;
    case type_t::MESSAGE:
      set_message(value, strlen(value));
      break;
    case type_t::REASON:
      set_reason(value, strlen(value));
      break;
    default:
      throw MessageException("Invalid attribute type");
  }
}

Message::Attribute::Attribute(type_t type, payload_t::client_count_t value)
    : type(type), length(sizeof(value)), payload() {
  assert(type == type_t::CLIENT_COUNT);
  set_client_count(value);
}

void Message::Attribute::validate() const {
  switch (type) {
    case type_t::USERNAME:
      if (length > SBCP_MAX_USERNAME_LENGTH) {
        throw MessageException("Username length exceeds maximum length");
      }
      break;
    case type_t::MESSAGE:
      if (length > SBCP_MAX_MESSAGE_LENGTH) {
        throw MessageException("Message length exceeds maximum length");
      }
      break;
    case type_t::REASON:
      if (length > SBCP_MAX_REASON_LENGTH) {
        throw MessageException("Reason length exceeds maximum length");
      }
      break;
    case type_t::CLIENT_COUNT:
      if (length != sizeof(payload_t::client_count_t)) {
        throw MessageException("Invalid client count length");
      }
      break;
    default:
      throw MessageException("Invalid attribute type");
  }
}

void Message::Attribute::set_username(const char* value,
                                      size_t length) noexcept {
  assert(type == type_t::USERNAME);
  this->length = length;
  if (this->length > SBCP_MAX_USERNAME_LENGTH) {
    this->length = SBCP_MAX_USERNAME_LENGTH;
  }
  memcpy(payload.username, value, this->length);
}

void Message::Attribute::set_message(const char* value,
                                     size_t length) noexcept {
  assert(type == type_t::MESSAGE);
  this->length = length;
  if (this->length > SBCP_MAX_MESSAGE_LENGTH) {
    this->length = SBCP_MAX_MESSAGE_LENGTH;
  }
  memcpy(payload.message, value, this->length);
}

void Message::Attribute::set_reason(const char* value, size_t length) noexcept {
  assert(type == type_t::REASON);
  this->length = length;
  if (this->length > SBCP_MAX_REASON_LENGTH) {
    this->length = SBCP_MAX_REASON_LENGTH;
  }
  memcpy(payload.reason, value, this->length);
}

void Message::Attribute::set_client_count(
    payload_t::client_count_t value) noexcept {
  assert(type == type_t::CLIENT_COUNT);
  this->length = sizeof(payload_t::client_count_t);
  payload.client_count = value;
}

void Message::validate() const {
  validate_version();
  constexpr size_t attribute_header_size =
      sizeof(Attribute::type_t) + sizeof(Attribute::length_t);
  switch (header.type) {
    case message_type_t::JOIN:
      // can only have one username attribute
      if (header.length > attribute_header_size + SBCP_MAX_USERNAME_LENGTH) {
        throw MessageException("JOIN message should not have payload");
      }
      break;
    case message_type_t::SEND:
      // can only have one message attribute
      if (header.length > attribute_header_size + SBCP_MAX_MESSAGE_LENGTH) {
        throw MessageException("SEND message payload overflow");
      }
      break;
    case message_type_t::FWD:
      // can only have one username and one message attribute
      if (header.length > 2 * (attribute_header_size) +
                              SBCP_MAX_USERNAME_LENGTH +
                              SBCP_MAX_MESSAGE_LENGTH) {
        throw MessageException("FWD message should have username and message");
      }
      break;
    case message_type_t::IDLE:
      // can only have one username attribute
      if (header.length > attribute_header_size + SBCP_MAX_USERNAME_LENGTH) {
        throw MessageException("IDLE message should have only username");
      }
      break;
    case message_type_t::ACK:
      // can have many username attributes and client count
      if (header.length > SBCP_MAX_PAYLOAD_LENGTH) {
        throw MessageException("ACK message size overflow");
      }
      break;
    case message_type_t::NAK:
      // can only have one reason attribute
      if (header.length > attribute_header_size + SBCP_MAX_REASON_LENGTH) {
        throw MessageException("NAK message should have only reason attribute");
      }
      break;
    case message_type_t::ONLINE:
      // can have many username attributes
      if (header.length > SBCP_MAX_PAYLOAD_LENGTH) {
        throw MessageException("ONLINE message size overflow");
      }
      break;
    case message_type_t::OFFLINE:
      // can have many username attributes
      if (header.length > SBCP_MAX_PAYLOAD_LENGTH) {
        throw MessageException("OFFLINE message size overflow");
      }
      break;
    default:
      throw MessageException("Invalid message type");
  }
}

void Message::validate_version() const {
  if (header.version != SBCP_VERSION) {
    throw MessageException("Invalid version");
  }
}

void Message::add_attribute(const attribute_t& attr) {
  validate();
  if (header.length + attr.size() > SBCP_MAX_PAYLOAD_LENGTH) {
    throw MessageException("Payload length exceeds maximum length");
  }
  memcpy(payload + header.length, &attr, attr.size());
  header.length += attr.size();
}

void Message::add_attribute(attribute_t::type_t type, const char* value,
                            size_t length) {
  assert(type == attribute_t::type_t::USERNAME ||
         type == attribute_t::type_t::MESSAGE ||
         type == attribute_t::type_t::REASON);
  add_attribute(attribute_t(type, value, length));
}

void Message::add_attribute(attribute_t::type_t type, const char* value) {
  assert(type == attribute_t::type_t::USERNAME ||
         type == attribute_t::type_t::MESSAGE ||
         type == attribute_t::type_t::REASON);
  add_attribute(attribute_t(type, value));
}

void Message::add_attribute(attribute_t::type_t type,
                            attribute_t::payload_t::client_count_t value) {
  assert(type == attribute_t::type_t::CLIENT_COUNT);
  add_attribute(attribute_t(type, value));
}

void Message::add_username(const char* value, size_t length) {
  add_attribute(attribute_t::type_t::USERNAME, value, length);
}

void Message::add_username(const char* value) {
  add_attribute(attribute_t::type_t::USERNAME, value);
}

void Message::add_message(const char* value, size_t length) {
  add_attribute(attribute_t::type_t::MESSAGE, value, length);
}

void Message::add_message(const char* value) {
  add_attribute(attribute_t::type_t::MESSAGE, value);
}

void Message::add_reason(const char* value, size_t length) {
  add_attribute(attribute_t::type_t::REASON, value, length);
}

void Message::add_reason(const char* value) {
  add_attribute(attribute_t::type_t::REASON, value);
}

void Message::add_client_count(attribute_t::payload_t::client_count_t value) {
  add_attribute(attribute_t::type_t::CLIENT_COUNT, value);
}

void Message::change_to_fwd(const char* username, size_t length) {
  assert(header.type == message_type_t::SEND);
  header.type = message_type_t::FWD;
  add_username(username, length);
}

void Message::change_to_fwd(const char* username) {
  assert(header.type == message_type_t::SEND);
  header.type = message_type_t::FWD;
  add_username(username);
}

void Message::change_to_fwd(const std::string& username) {
  change_to_fwd(username.c_str(), username.length());
}

std::vector<std::string> Message::get_usernames() const {
  std::vector<std::string> usernames;
  for (const auto& attr : *this) {
    if (attr.get_type() == attribute_t::type_t::USERNAME) {
      usernames.push_back(attr.get_username());
    }
  }
  if (usernames.empty()) {
    throw MessageException("Username attribute not found");
  }
  return usernames;
}

std::string Message::get_username() const { return get_usernames().front(); }

std::string Message::get_message() const {
  for (const auto& attr : *this) {
    if (attr.get_type() == attribute_t::type_t::MESSAGE) {
      return attr.get_message();
    }
  }
  throw MessageException("Message attribute not found");
}

std::string Message::get_reason() const {
  for (const auto& attr : *this) {
    if (attr.get_type() == attribute_t::type_t::REASON) {
      return attr.get_reason();
    }
  }
  throw MessageException("Reason attribute not found");
}

attribute_t::payload_t::client_count_t Message::get_client_count() const {
  for (const auto& attr : *this) {
    if (attr.get_type() == attribute_t::type_t::CLIENT_COUNT) {
      return attr.get_client_count();
    }
  }
  throw MessageException("Client count attribute not found");
}

const Message::attribute_t& Message::operator[](size_t idx) const {
  size_t offset = 0;
  do {
    const attribute_t* attr =
        reinterpret_cast<const attribute_t*>(payload + offset);
    attr->validate();
    if (idx == 0) {
      return *attr;
    }
    offset += attr->size();
    --idx;
  } while (offset < header.length);
  throw std::out_of_range("Index out of range");
}

std::ostream& operator<<(std::ostream& os, const attribute_type_t type) {
  switch (type) {
    case attribute_type_t::USERNAME:
      os << "USERNAME";
      break;
    case attribute_type_t::MESSAGE:
      os << "MESSAGE";
      break;
    case attribute_type_t::REASON:
      os << "REASON";
      break;
    case attribute_type_t::CLIENT_COUNT:
      os << "CLIENT_COUNT";
      break;
    default:
      os << "UNKNOWN";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Message::Attribute& attr) {
  os << "Attribute: " << attr.get_type() << " (length: " << attr.get_length()
     << ") - ";
  switch (attr.get_type()) {
    case Message::Attribute::type_t::USERNAME:
      os.write(attr.get_username(), attr.get_length());
      break;
    case Message::Attribute::type_t::MESSAGE:
      os.write(attr.get_message(), attr.get_length());
      break;
    case Message::Attribute::type_t::REASON:
      os.write(attr.get_reason(), attr.get_length());
      break;
    case Message::Attribute::type_t::CLIENT_COUNT:
      os << attr.get_client_count();
      break;
    default:
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const message_type_t type) {
  switch (type) {
    case message_type_t::JOIN:
      os << "JOIN";
      break;
    case message_type_t::SEND:
      os << "SEND";
      break;
    case message_type_t::FWD:
      os << "FWD";
      break;
    case message_type_t::ACK:
      os << "ACK";
      break;
    case message_type_t::NAK:
      os << "NAK";
      break;
    case message_type_t::ONLINE:
      os << "ONLINE";
      break;
    case message_type_t::OFFLINE:
      os << "OFFLINE";
      break;
    case message_type_t::IDLE:
      os << "IDLE";
      break;
    default:
      os << "UNKNOWN";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Message& msg) {
  os << "Message: " << msg.get_type() << " (version: " << msg.get_version()
     << ", length: " << msg.get_length() << ")";
  for (const auto& attr : msg) {
    os << std::endl << "  " << attr;
  }
  return os;
}

}  // namespace sbcp