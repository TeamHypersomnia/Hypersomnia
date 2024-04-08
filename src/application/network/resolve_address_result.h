#pragma once
#include "application/network/resolve_result_type.h"
#include "3rdparty/yojimbo/netcode/netcode.h"

struct resolve_address_result {
	netcode_address_t addr;
	std::string host;

	resolve_result_type result = resolve_result_type::OK;

	std::string report() const;
};
