#include "sbcp.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

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
      throw std::runtime_error("Invalid attribute type");
  }
}

Message::Attribute::Attribute(type_t type, const char* value)
    : type(type), length(0), payload() {
  switch (type) {
    case type_t::USERNAME:
      set_username(value, std::strlen(value));
      break;
    case type_t::MESSAGE:
      set_message(value, std::strlen(value));
      break;
    case type_t::REASON:
      set_reason(value, std::strlen(value));
      break;
    default:
      throw std::runtime_error("Invalid attribute type");
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
        throw std::runtime_error("Username length exceeds maximum length");
      }
      break;
    case type_t::MESSAGE:
      if (length > SBCP_MAX_MESSAGE_LENGTH) {
        throw std::runtime_error("Message length exceeds maximum length");
      }
      break;
    case type_t::REASON:
      if (length > SBCP_MAX_REASON_LENGTH) {
        throw std::runtime_error("Reason length exceeds maximum length");
      }
      break;
    case type_t::CLIENT_COUNT:
      if (length != sizeof(payload_t::client_count_t)) {
        throw std::runtime_error("Invalid client count length");
      }
      break;
    default:
      throw std::runtime_error("Invalid attribute type");
  }
}

void Message::Attribute::set_username(const char* value,
                                      size_t length) noexcept {
  assert(type == type_t::USERNAME);
  this->length = length;
  if (this->length > SBCP_MAX_USERNAME_LENGTH) {
    this->length = SBCP_MAX_USERNAME_LENGTH;
  }
  std::memcpy(payload.username, value, this->length);
}

void Message::Attribute::set_message(const char* value,
                                     size_t length) noexcept {
  assert(type == type_t::MESSAGE);
  this->length = length;
  if (this->length > SBCP_MAX_MESSAGE_LENGTH) {
    this->length = SBCP_MAX_MESSAGE_LENGTH;
  }
  std::memcpy(payload.message, value, this->length);
}

void Message::Attribute::set_reason(const char* value, size_t length) noexcept {
  assert(type == type_t::REASON);
  this->length = length;
  if (this->length > SBCP_MAX_REASON_LENGTH) {
    this->length = SBCP_MAX_REASON_LENGTH;
  }
  std::memcpy(payload.reason, value, this->length);
}

void Message::Attribute::set_client_count(
    payload_t::client_count_t value) noexcept {
  assert(type == type_t::CLIENT_COUNT);
  this->length = sizeof(payload_t::client_count_t);
  payload.client_count = value;
}

void Message::validate() const {
  validate_version();
  if (length > SBCP_MAX_PAYLOAD_LENGTH) {
    throw std::runtime_error("Payload length exceeds maximum length");
  }
}

void Message::validate_version() const {
  if (version != SBCP_VERSION) {
    throw std::runtime_error("Invalid version");
  }
}

void Message::add_attribute(const attribute_t& attr) {
  validate();
  if (length + attr.size() > SBCP_MAX_PAYLOAD_LENGTH) {
    throw std::runtime_error("Payload length exceeds maximum length");
  }
  std::memcpy(payload + length, &attr, attr.size());
  length += attr.size();
}

void Message::add_attribute(attribute_t::type_t type, const char* value,
                            size_t length) {
  add_attribute(attribute_t(type, value, length));
}

void Message::add_attribute(attribute_t::type_t type, const char* value) {
  add_attribute(attribute_t(type, value));
}

void Message::add_attribute(attribute_t::type_t type,
                            attribute_t::payload_t::client_count_t value) {
  add_attribute(attribute_t(type, value));
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
  } while (offset < length);
  throw std::out_of_range("Index out of range");
}

}  // namespace sbcp