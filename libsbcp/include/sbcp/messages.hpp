#pragma once

#include <string>
#include <vector>

#include "sbcp/sbcp.hpp"

namespace sbcp {

message_t JOIN(const char* username, size_t length);
message_t JOIN(const char* username);
message_t JOIN(std::string username);

message_t SEND(const char* message, size_t length);
message_t SEND(const char* message);
message_t SEND(std::string message);

message_t FWD(const char* username, size_t username_length, const char* message,
              size_t message_length);
message_t FWD(const char* username, const char* message);
message_t FWD(std::string username, std::string message);

message_t ACK(message_t::attribute_t::payload_t::client_count_t client_count,
              std::vector<std::string> usernames);

message_t NAK(const char* reason, size_t length);
message_t NAK(const char* reason);
message_t NAK(std::string reason);

message_t ONLINE(const char* username, size_t length);
message_t ONLINE(const char* username);
message_t ONLINE(std::string username);

message_t OFFLINE(const char* username, size_t length);
message_t OFFLINE(const char* username);
message_t OFFLINE(std::string username);

message_t IDLE(const char* username, size_t length);
message_t IDLE(const char* username);
message_t IDLE(std::string username);
message_t IDLE();

}  // namespace sbcp
