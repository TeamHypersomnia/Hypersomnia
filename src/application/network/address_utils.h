#pragma once
#include <future>
#include <optional>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/network/resolve_address_result.h"
#include "augs/network/port_type.h"
#include "augs/filesystem/path.h"
#include "application/setups/client/client_connect_string.h"

namespace yojimbo {
	class Address;
}

struct host_with_default_port;

std::string add_ws_preffix(const std::string& websocket_url);
bool is_internal_webrtc_address(const netcode_address_t& t);
netcode_address_t make_internal_webrtc_address(unsigned short client_identifier);

std::string ToString(const yojimbo::Address&);
std::string ToString(const netcode_address_t&);

bool is_internal(const netcode_address_t& address);

yojimbo::Address to_yojimbo_addr(const netcode_address_t& t);
netcode_address_t to_netcode_addr(const yojimbo::Address& t);

std::optional<augs::path_type> find_demo_path(const client_connect_string&);
std::optional<netcode_address_t> find_netcode_addr(const client_connect_string& ip);
std::optional<netcode_address_t> find_netcode_addr(const host_with_default_port& in);

resolve_address_result resolve_address(const host_with_default_port& in);
std::future<resolve_address_result> async_resolve_address(const host_with_default_port& in);

#if BUILD_NATIVE_SOCKETS
std::optional<netcode_address_t> get_internal_network_address();
std::future<std::optional<netcode_address_t>> async_get_internal_network_address();
#endif
