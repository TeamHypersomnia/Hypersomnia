#pragma once
#include <future>
#include <optional>
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/network/resolve_address_result.h"
#include "augs/network/port_type.h"
#include "augs/filesystem/path.h"
#include "application/setups/client/client_start_input.h"

namespace yojimbo {
	class Address;
}

struct address_and_port;

std::string ToString(const yojimbo::Address&);
std::string ToString(const netcode_address_t&);

yojimbo::Address to_yojimbo_addr(const netcode_address_t& t);
netcode_address_t to_netcode_addr(const yojimbo::Address& t);

std::optional<augs::path_type> find_demo_path(const client_connect_string&);
std::optional<netcode_address_t> find_netcode_addr(const client_connect_string& ip);
std::optional<netcode_address_t> find_netcode_addr(const address_and_port& in);

resolve_address_result resolve_address(const address_and_port& in);
std::future<resolve_address_result> async_resolve_address(const address_and_port& in);

std::optional<netcode_address_t> get_internal_network_address();
std::future<std::optional<netcode_address_t>> async_get_internal_network_address();
