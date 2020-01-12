#pragma once
#include <future>
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/network/resolve_address_result.h"

namespace yojimbo {
	class Address;
}

struct address_and_port;

std::string ToString(const yojimbo::Address&);
std::string ToString(const netcode_address_t&);

yojimbo::Address to_yojimbo_addr(const netcode_address_t& t);
netcode_address_t to_netcode_addr(const yojimbo::Address& t);

resolve_address_result resolve_address(const address_and_port& in);
std::future<resolve_address_result> async_resolve_address(const address_and_port& in);
