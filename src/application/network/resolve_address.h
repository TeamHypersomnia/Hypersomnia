#pragma once
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/network/resolve_result_type.h"

struct resolve_address_result {
	yojimbo::Address addr;
	std::string host;

	resolve_result_type result = resolve_result_type::OK;

	void report() const;
};

std::string ToString(const yojimbo::Address&);
std::string ToString(const netcode_address_t&);

yojimbo::Address to_yojimbo_addr(const netcode_address_t& t);
netcode_address_t to_netcode_addr(const yojimbo::Address& t);

resolve_address_result resolve_address(const address_and_port& in);
