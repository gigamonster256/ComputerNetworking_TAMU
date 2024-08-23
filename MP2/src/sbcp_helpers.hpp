#ifndef _SBCP_HELPERS_HPP_
#define _SBCP_HELPERS_HPP_

#include "sbcp.hpp"

namespace sbcp {

void print_attr_type(const attribute_type_t type);
void print_attr(const attribute_t& attr);
void print_message_type(const message_type_t type);
void print_message(const message_t& msg);

}  // namespace sbcp

#endif