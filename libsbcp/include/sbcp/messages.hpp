#ifndef _SBCP_MESSAGES_HPP_
#define _SBCP_MESSAGES_HPP_

#include <string>
#include <vector>

#include "sbcp/sbcp.hpp"

namespace sbcp {

const message_t JOIN(const char* username, size_t length);
const message_t JOIN(const char* username);
const message_t JOIN(std::string username);

const message_t SEND(const char* message, size_t length);
const message_t SEND(const char* message);
const message_t SEND(std::string message);

const message_t FWD(const char* username, size_t username_length,
                    const char* message, size_t message_length);
const message_t FWD(const char* username, const char* message);
const message_t FWD(std::string username, std::string message);

const message_t ACK(
    message_t::attribute_t::payload_t::client_count_t client_count,
    std::vector<std::string> usernames);

const message_t NAK(const char* reason, size_t length);
const message_t NAK(const char* reason);
const message_t NAK(std::string reason);

const message_t ONLINE(const char* username, size_t length);
const message_t ONLINE(const char* username);
const message_t ONLINE(std::string username);

const message_t OFFLINE(const char* username, size_t length);
const message_t OFFLINE(const char* username);
const message_t OFFLINE(std::string username);

const message_t IDLE(const char* username, size_t length);
const message_t IDLE(const char* username);
const message_t IDLE(std::string username);
const message_t IDLE();

}  // namespace sbcp

#endif