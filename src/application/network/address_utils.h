#pragma once
#include <future>
#include <optional>
#include <vector>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/network/resolve_address_result.h"
#include "augs/network/port_type.h"
#include "augs/filesystem/path.h"
#include "application/setups/client/client_connect_string.h"

namespace yojimbo {
	class Address;
}

struct host_with_default_port;
struct netcode_socket_t;

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

/*
	Pasted text that is surely an address: what looks like an IP (digits, dots and colons, or [ipv6]) only a valid
	one, else a hostname of letters, digits, dots and dashes with an optional :port.
*/

bool is_pasted_address(const std::string& text);

#if BUILD_NATIVE_SOCKETS
resolve_address_result resolve_address(const host_with_default_port& in);
std::future<resolve_address_result> async_resolve_address(const host_with_default_port& in);

std::optional<netcode_address_t> get_internal_network_address();
std::future<std::optional<netcode_address_t>> async_get_internal_network_address();

/*
	Finding the servers on the local network (see browse_servers_gui.cpp): 255.255.255.255 and each IPv4 interface's own
	broadcast address - the first alone may go out through one interface only, or nowhere without a default route.
*/

bool enable_broadcast(netcode_socket_t& socket);
std::vector<netcode_address_t> get_broadcast_addresses();
#endif
