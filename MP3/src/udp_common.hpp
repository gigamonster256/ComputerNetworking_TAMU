#ifndef _UDP_COMMON_HPP_
#define _UDP_COMMON_HPP_

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

static constexpr uint32_t HANDSHAKE_PHASE1 = 0xBA5EBA11;
static constexpr uint32_t HANDSHAKE_PHASE2 = 0xFACEB00C;

enum ip_version { IPv4, IPv6 };

#endif