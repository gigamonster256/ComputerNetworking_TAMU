#include "messages.hpp"

#include <cassert>
#include <cstring>

namespace sbcp {

const message_t JOIN(const char* username, size_t length) {
  message_t message(message_type_t::JOIN);
  message.add_username(username, length);
  return message;
}

const message_t JOIN(const char* username) {
  return JOIN(username, std::strlen(username));
}

const message_t JOIN(std::string username) {
  return JOIN(username.c_str(), username.length());
}

const message_t SEND(const char* message, size_t length) {
  message_t msg(message_type_t::SEND);
  msg.add_message(message, length);
  return msg;
}

const message_t SEND(const char* message) {
  return SEND(message, std::strlen(message));
}

const message_t SEND(std::string message) {
  return SEND(message.c_str(), message.length());
}

const message_t FWD(const char* username, size_t username_length,
                    const char* message, size_t message_length) {
  message_t msg(message_type_t::FWD);
  msg.add_username(username, username_length);
  msg.add_message(message, message_length);
  return msg;
}

const message_t FWD(const char* username, const char* message) {
  return FWD(username, std::strlen(username), message, std::strlen(message));
}

const message_t FWD(std::string username, std::string message) {
  return FWD(username.c_str(), username.length(), message.c_str(),
             message.length());
}

const message_t ACK(
    message_t::attribute_t::payload_t::client_count_t client_count,
    std::vector<std::string> usernames) {
  // the client count should be inclusive of the requestor
  // while the username list should be exclusive
  assert(usernames.size() + 1 == client_count);
  message_t msg(message_type_t::ACK);
  msg.add_client_count(client_count);
  for (const auto& username : usernames) {
    msg.add_username(username.c_str(), username.length());
  }
  return msg;
}

const message_t NAK(const char* reason, size_t length) {
  message_t msg(message_type_t::NAK);
  msg.add_reason(reason, length);
  return msg;
}

const message_t NAK(const char* reason) {
  return NAK(reason, std::strlen(reason));
}

const message_t NAK(std::string reason) {
  return NAK(reason.c_str(), reason.length());
}

const message_t ONLINE(const char* username, size_t length) {
  message_t msg(message_type_t::ONLINE);
  msg.add_username(username, length);
  return msg;
}

const message_t ONLINE(const char* username) {
  return ONLINE(username, std::strlen(username));
}

const message_t ONLINE(std::string username) {
  return ONLINE(username.c_str(), username.length());
}

const message_t OFFLINE(const char* username, size_t length) {
  message_t msg(message_type_t::OFFLINE);
  msg.add_username(username, length);
  return msg;
}

const message_t OFFLINE(const char* username) {
  return OFFLINE(username, std::strlen(username));
}

const message_t OFFLINE(std::string username) {
  return OFFLINE(username.c_str(), username.length());
}

const message_t IDLE(const char* username, size_t length) {
  message_t msg(message_type_t::IDLE);
  msg.add_username(username, length);
  return msg;
}

const message_t IDLE(const char* username) {
  return IDLE(username, std::strlen(username));
}

const message_t IDLE(std::string username) {
  return IDLE(username.c_str(), username.length());
}

const message_t IDLE() { return message_t(message_type_t::IDLE); }

}  // namespace sbcp