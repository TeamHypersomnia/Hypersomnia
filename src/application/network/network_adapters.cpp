#include <stdexcept>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include "application/network/address_utils.h"
#include "application/network/server_adapter.h"
#include "application/network/client_adapter.h"

#include "augs/network/network_types.h"
#include "augs/readwrite/memory_stream.h"
#include "hypersomnia_version.h"

#include "application/network/net_serialize.h"
#include "application/network/identity_cert.h"
#include "application/setups/server/server_slots.h"
#include "3rdparty/yojimbo/netcode/netcode.c"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/templates/thread_templates.h"

#undef SendMessage
#undef SetPort

static_assert(max_incoming_connections_v == yojimbo::MaxClients);
static_assert(max_netcode_connections_v == yojimbo::MaxClients);

namespace augs {
	double steady_secs();
}

void GameAdapter::OnServerClientConnected(const int clientIndex) {
	if (m_server != nullptr) {
		m_server->client_connected(static_cast<client_id_type>(clientIndex));
	}
}

void GameAdapter::OnServerClientDisconnected(const int clientIndex) {
	if (m_server != nullptr) {
		m_server->client_disconnected(static_cast<client_id_type>(clientIndex));
	}
}

bool GameAdapter::AllowHandshake(const yojimbo::Address& from) {
	if (m_server != nullptr && m_server->allow_handshake) {
		return m_server->allow_handshake(to_netcode_addr(from));
	}

	return true;
}

bool GameAdapter::AcceptWhileRefusing(const yojimbo::Address& from, const uint8_t* binding, const uint8_t* session_signature, const uint8_t* user_data) {
	if (m_server != nullptr && m_server->accept_while_refusing) {
		return m_server->accept_while_refusing(to_netcode_addr(from), binding, session_signature, user_data);
	}

	return false;
}

void server_adapter::stop() {
	server.Stop();
}

void server_adapter::client_connected(const client_id_type id) {
	pending_events.push_back({ id, true });
}

void server_adapter::client_disconnected(const client_id_type id) {
	pending_events.push_back({ id, false });
}

game_connection_config::game_connection_config() {
	numChannels = static_cast<int>(game_channel_type::COUNT);

	/* We manage the timeout ourselves. */
	timeout = -1;

	{
		auto& solvable_stream = channel[static_cast<int>(game_channel_type::RELIABLE_MESSAGES)];
		solvable_stream.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		solvable_stream.maxBlockSize = max_block_size_v;
		solvable_stream.sentPacketBufferSize = 1024 * 2;
		solvable_stream.maxMessagesPerPacket = 256;
		solvable_stream.messageResendTime = 0.f;
		solvable_stream.messageSendQueueSize = 1024 * 8;
		solvable_stream.messageReceiveQueueSize = 1024 * 8;
		solvable_stream.blockFragmentSize = block_fragment_size_v;
		solvable_stream.blockFragmentResendTime = 0.15f;

		ackedPacketsBufferSize = 1024 * 4;
		receivedPacketsBufferSize = 1024 * 4;
	}

	{
		auto& stats = channel[static_cast<int>(game_channel_type::VOLATILE_STATISTICS)];
		stats.type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
	}

	serverPerClientMemory = 14 * 1024 * 1024;
	clientMemory = 1024 * 1024 * 50;

	networkSimulator = true;

	set_max_packet_size(max_packet_size_v);

}

void game_connection_config::set_max_packet_size(const unsigned s) {
	protocolId = DEFAULT_GAME_PORT_V;

	maxPacketSize = s;
    maxPacketFragments = (int) ceil( maxPacketSize / packetFragmentSize );
}

game_connection_config::game_connection_config(const augs::server_listen_input& in) : game_connection_config() {
	(void)in;
#if 0
	set_max_packet_size(in.max_packet_size);
#endif
}

const uint8_t* server_adapter::get_client_user_data(const client_id_type& id) const {
	static_assert(identity_cert::blob_bytes == NETCODE_USER_DATA_BYTES);

	if (!server.IsClientConnected(id)) {
		return nullptr;
	}

	return server.GetClientUserData(id);
}

const uint8_t* server_adapter::get_client_binding(const client_id_type& id) const {
	if (!server.IsClientConnected(id)) {
		return nullptr;
	}

	return server.GetClientBinding(id);
}

const uint8_t* server_adapter::get_client_session_signature(const client_id_type& id) const {
	if (!server.IsClientConnected(id)) {
		return nullptr;
	}

	return server.GetClientSessionSignature(id);
}

bool server_adapter::is_running() const {
	return server.IsRunning();
}

bool server_adapter::is_client_connected(const client_id_type& id) const {
	return server.IsClientConnected(id);
}

bool server_adapter::can_send_message(const client_id_type& id, const game_channel_type& channel) const {
	return server.CanSendMessage(id, static_cast<channel_id_type>(channel));
}

bool server_adapter::has_messages_to_send(const client_id_type& id, const game_channel_type& channel) const {
	return server.HasMessagesToSend(id, static_cast<channel_id_type>(channel));
}

std::string resolve_address_result::report() const {
	switch (result) {
		case resolve_result_type::OK:
			return typesafe_sprintf("Successfully resolved %x to %x", host, ::ToString(addr));
		case resolve_result_type::COULDNT_RESOLVE_HOST:
			return typesafe_sprintf("Couldn't resolve %x: host unreachable.", host);
		case resolve_result_type::INVALID_ADDRESS:
			return typesafe_sprintf("Couldn't resolve %x: the address is invalid.", host);
		case resolve_result_type::IPV6_NOT_SUPPORTED:
			return typesafe_sprintf("%x is an IPv6 address - not supported.", host);
		default:
			return typesafe_sprintf("Couldn't resolve %x: unknown error.", host);
	}
}

bool operator==(const netcode_address_t& a, const netcode_address_t& b) {
	auto aa = a;
	auto bb = b;
	return 1 == netcode_address_equal(&aa, &bb);
}

bool operator!=(const netcode_address_t& a, const netcode_address_t& b) {
	auto aa = a;
	auto bb = b;
	return 0 == netcode_address_equal(&aa, &bb);
}

bool host_equal(const netcode_address_t& a, const netcode_address_t& b) {
	auto aa = a;
	auto bb = b;
	aa.port = 0;
	bb.port = 0;

	return 1 == netcode_address_equal(&aa, &bb);
}

std::string ToString(const yojimbo::Address& addr) {
	char buffer[256];
	addr.ToString(buffer, sizeof(buffer));

	return buffer;
}

std::string ToString(const netcode_address_t& addr) {
	auto a = addr;
	char buffer[256];
	netcode_address_to_string(&a, buffer);

	return buffer;
}

bool try_fire_interval(const double interval, net_time_t& when_last, const double current_time) {
	if (when_last < 0.0 || current_time - when_last >= interval) {
		when_last = current_time;
		return true;
	}

	return false;
}

bool try_fire_interval(const double interval, net_time_t& when_last) {
	return try_fire_interval(interval, when_last, augs::steady_secs());
}

bool auxiliary_command_function(void* context, struct netcode_address_t* from, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<server_adapter*>(context);
	return adapter->auxiliary_command_callback(*from, reinterpret_cast<const std::byte*>(packet), bytes);
}

void send_packet_override(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<yojimbo::Server*>(context)->GetParent();
	adapter->send_packet_override(*to, reinterpret_cast<const std::byte*>(packet), bytes);
}

bool aux_send_packet(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<yojimbo::Server*>(context)->GetParent();
	return adapter->send_packet_override(*to, reinterpret_cast<const std::byte*>(packet), bytes);
}

int receive_packet_override(void* context, netcode_address_t* from, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<yojimbo::Server*>(context)->GetParent();
	return adapter->receive_packet_override(*from, reinterpret_cast<std::byte*>(packet), bytes);
}

server_adapter::server_adapter(
	const augs::server_listen_input& in, 
	const bool is_integrated, 
	auxiliary_command_callback_type auxiliary_command_callback,
	send_packet_override_type send_packet_override,
	receive_packet_override_type receive_packet_override,
	allow_handshake_type allow_handshake,
	const bool is_webrtc_only,
	const augs::ed25519_seed_type& server_key
) :
	privateKey([&server_key]() {
		static_assert(std::tuple_size_v<augs::ed25519_secret_key_type> == yojimbo::ServerSecretKeyBytes);

		if (const auto secret = augs::ed25519_secret_key(server_key)) {
			return *secret;
		}

		/*
			Never a server on a half-made key.
		*/

		throw std::runtime_error("Could not derive the server's secret key.");
	}()),
	connection_config(in),
	adapter(this),
	server(
		yojimbo_allocator, 
		privateKey.data(), 
		yojimbo::Address(in.ip.c_str(), in.port), 
		connection_config, 
		adapter, 
		augs::high_precision_secs(),
		this
	),
	auxiliary_command_callback(auxiliary_command_callback),
	send_packet_override(send_packet_override),
	receive_packet_override(receive_packet_override),
	allow_handshake(allow_handshake)
{
	auto slots = in.slots;

	if (is_integrated) {
		slots -= 1;
	}

	/*
		A few connections more than the game slots: the game decides who gets a slot (server_setup::admit_or_refuse) -
		e.g. a player back after a crash replaces the connection that has not timed out yet, even on a full server,
		and a full server answers why instead of netcode saying nothing. And the admins' places (server_slots.h).
	*/

	const auto netcode_slots = std::min(std::max(1, slots) + spare_connections_v + reserved_admin_places_v + reserved_spectator_places_v, static_cast<int>(yojimbo::MaxClients));

	LOG("Starting yojimbo::Server with %x connections for %x slots.", netcode_slots, std::max(1, slots));
	server.Start(netcode_slots);
	LOG("Server address is %x", ToString(server.GetAddress()));

	if (auto detail = server.GetServerDetail()) {
		detail->config.auxiliary_command_function = auxiliary_command_function;
		detail->config.auxiliary_command_context = this;
		/*
			We send our own disconnect messages with e.g. kick reasons included.
			netcode's disconnect packets could arrive first and drop the reason.
		*/
		detail->config.disable_disconnect_packets = 1;

		if (is_webrtc_only) {
			detail->config.override_send_and_receive = 1;
			detail->config.send_packet_override = ::send_packet_override;
			detail->config.receive_packet_override = ::receive_packet_override;
		}
		else {
			detail->config.aux_send_packet = ::aux_send_packet;
			detail->config.aux_receive_packet = ::receive_packet_override;
		}
	}
}

void server_adapter::disconnect_client(const client_id_type& id) {
	server.DisconnectClient(id);
	erase_if(
		pending_events,
		[id](const auto& e) {
			if (e.client_id == id && e.connected == false) {
				LOG("Remove disconnection event from pending_events since the caller will already manage it.");

				return true;
			}

			return false;
		}
	);
}

void server_adapter::send_packets() {
	server.SendPackets();
}

bool client_auxiliary_command_function(void* context, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<client_adapter*>(context);
	return adapter->auxiliary_command_callback(reinterpret_cast<std::byte*>(packet), bytes);
}

client_adapter::client_adapter(
	const std::optional<port_type> preferred_binding_port,
	client_auxiliary_command_callback_type auxiliary_command_callback,
	send_packet_override_type send_packet_override,
	receive_packet_override_type receive_packet_override
) :
	connection_config(),
	adapter(nullptr),
	client(
		yojimbo_allocator, 
		preferred_binding_port ? yojimbo::Address("0.0.0.0", *preferred_binding_port) : yojimbo::Address("0.0.0.0"), 
		connection_config, 
		adapter, 
		augs::high_precision_secs(),
		this
	),
	auxiliary_command_callback(auxiliary_command_callback),
	send_packet_override(send_packet_override),
	receive_packet_override(receive_packet_override)
{
}

std::optional<unsigned long> get_trailing_number(const std::string& s);
std::string cut_trailing_number(const std::string& s);

std::string add_ws_preffix(const std::string& websocket_url) {
	const auto preffix = websocket_url.find("://") == std::string::npos ? "ws://" : "";
	return std::string(preffix) + websocket_url;
}

netcode_address_t make_internal_webrtc_address(unsigned short client_identifier) {
	netcode_address_t address;
	address.type = NETCODE_ADDRESS_IPV4;
	address.port = client_identifier;
	address.data.ipv4[0] = 127;
	address.data.ipv4[1] = 255;
	address.data.ipv4[2] = 255;
	address.data.ipv4[3] = 255;

	return address;
}

bool is_internal_webrtc_address(const netcode_address_t& address) {
	if (address.type == NETCODE_ADDRESS_IPV4) {
		const auto ip = address.data.ipv4;

		return ip[0] == 127 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255;
	}

	return false;
}

netcode_address_t to_netcode_addr(sockaddr_in* addr_ipv4) {
	netcode_address_t out;
	out.type = NETCODE_ADDRESS_IPV4;

	out.data.ipv4[0] = (uint8_t) ( ( addr_ipv4->sin_addr.s_addr & 0x000000FF ) );
	out.data.ipv4[1] = (uint8_t) ( ( addr_ipv4->sin_addr.s_addr & 0x0000FF00 ) >> 8 );
	out.data.ipv4[2] = (uint8_t) ( ( addr_ipv4->sin_addr.s_addr & 0x00FF0000 ) >> 16 );
	out.data.ipv4[3] = (uint8_t) ( ( addr_ipv4->sin_addr.s_addr & 0xFF000000 ) >> 24 );

	out.port = ntohs( addr_ipv4->sin_port );

	return out;
}

netcode_address_t to_netcode_addr(sockaddr_in6* addr_ipv6) {
	netcode_address_t out;
	out.type = NETCODE_ADDRESS_IPV6;

	int i;

	for ( i = 0; i < 8; ++i )
	{
		out.data.ipv6[i] = ntohs( ( (uint16_t*) &addr_ipv6->sin6_addr ) [i] );
	}

	out.port = ntohs( addr_ipv6->sin6_port );

	return out;
}

std::optional<netcode_address_t> to_netcode_addr(addrinfo* p) {
	if (p->ai_family == AF_INET) {
		return to_netcode_addr((struct sockaddr_in *)p->ai_addr);

	} 
	else if (p->ai_family == AF_INET6) { 
		return to_netcode_addr((struct sockaddr_in6 *)p->ai_addr);
	}

	return std::nullopt;
}

std::optional<netcode_address_t> hostname_to_netcode_address_t(const std::string& hostname, bool accept_ip4, bool accept_ip6) {
	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname.c_str(), NULL, &hints, &res) != 0) {
		return std::nullopt;
	}

	std::optional<netcode_address_t> from;

	for (p = res;p != NULL; p = p->ai_next) {
		const auto out = to_netcode_addr(p);

		if (out == std::nullopt) {
			continue;
		}

		if (accept_ip4 && out->type == NETCODE_ADDRESS_IPV4) 
		{
			from = out;
			break;
		}

		if (accept_ip6 && out->type == NETCODE_ADDRESS_IPV6) 
		{
			from = out;
			break;
		}
	}

	freeaddrinfo(res);

	return from;
}

std::optional<augs::path_type> find_demo_path(const client_connect_string& str) {
	if (begins_with(str, demo_address_preffix_v)) {
		auto path = str;
		cut_preffix(path, demo_address_preffix_v);

		return path;
	}

	return {};
}

std::optional<netcode_address_t> find_netcode_addr(const client_connect_string& str) {
	host_with_default_port in;
	in.address = str;
	in.default_port = DEFAULT_GAME_PORT_V;

	return find_netcode_addr(in);
}

netcode_address_t to_netcode_addr(const yojimbo::Address& t) {
	using namespace yojimbo;

	netcode_address_t out;

	out.port = t.GetPort();

	if (t.GetType() == AddressType::ADDRESS_IPV4) {
		out.type = NETCODE_ADDRESS_IPV4;
		const auto n = std::size(out.data.ipv4);
		std::copy(t.GetAddress4(), t.GetAddress4() + n, out.data.ipv4);
	}

	if (t.GetType() == AddressType::ADDRESS_IPV6) {
		out.type = NETCODE_ADDRESS_IPV6;
		const auto n = std::size(out.data.ipv6);
		std::copy(t.GetAddress6(), t.GetAddress6() + n, out.data.ipv6);
	}

	return out;
}

yojimbo::Address to_yojimbo_addr(const netcode_address_t& t) {
	if (t.type == NETCODE_ADDRESS_IPV4) {
		return yojimbo::Address(t.data.ipv4, t.port);
	}

	return yojimbo::Address(t.data.ipv6, t.port);
}

bool is_pasted_address(const std::string& text) {
	const auto first = text.find_first_not_of(" \t\r\n");
	const auto last = text.find_last_not_of(" \t\r\n");

	if (first == std::string::npos) {
		return false;
	}

	const auto in = text.substr(first, last - first + 1);

	const bool ip_like = in[0] == '[' || in.find_first_not_of("0123456789.:") == std::string::npos;

	if (ip_like) {
		return ::find_netcode_addr(client_connect_string(in)).has_value();
	}

	const auto colon = in.rfind(':');
	const auto host = colon == std::string::npos ? in : in.substr(0, colon);
	const auto port = colon == std::string::npos ? std::string() : in.substr(colon + 1);

	if (colon != std::string::npos && (port.empty() || port.size() > 5 || port.find_first_not_of("0123456789") != std::string::npos)) {
		return false;
	}

	if (host.empty() || host.front() == '.' || host.front() == '-') {
		return false;
	}

	for (const auto c : host) {
		const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-';

		if (!ok) {
			return false;
		}
	}

	return true;
}

std::optional<netcode_address_t> find_netcode_addr(const host_with_default_port& in) {
	const auto& input = in.address;
	const auto default_port = in.default_port;

	if (input.empty()) {
		return std::nullopt;
	}

	{
		netcode_address_t out;

		if (netcode_parse_address(input.c_str(), &out) == NETCODE_OK) {
			if (out.port == 0) {
				out.port = default_port;
			}

			return out;
		}
	}

	return std::nullopt;
}

#if BUILD_NATIVE_SOCKETS
std::future<resolve_address_result> async_resolve_address(const host_with_default_port& in) {
	return launch_async([in]() { return resolve_address(in); });
}

resolve_address_result resolve_address(const host_with_default_port& in) {
	const auto& input = in.address;
	const auto default_port = in.default_port;

	if (input.empty()) {
		resolve_address_result out;
		out.host = input;
		out.result = resolve_result_type::INVALID_ADDRESS;
		return out;
	}

	{
		resolve_address_result out;

		if (netcode_parse_address(input.c_str(), &out.addr) == NETCODE_OK) {
			if (out.addr.port == 0) {
				out.addr.port = default_port;
			}

			out.host = input.c_str();
			return out;
		}
	}

	auto no_port = [&]() {
		auto result = cut_trailing_number(std::string(input));
		
		if (result.size() > 0 && result.back() == ':') {
			result.pop_back();
		}

		if (result.size() > 0 && result.back() == ']') {
			result.pop_back();
		}

		if (result.size() > 0 && result.front() == '[') {
			result.erase(result.begin());
		}

		return result;
	}();

	auto resolved_net_addr = hostname_to_netcode_address_t(no_port);

	if (resolved_net_addr == std::nullopt) {
		resolve_address_result out;

		out.result = resolve_result_type::COULDNT_RESOLVE_HOST;
		out.host = no_port;
		return out;
	}

	resolved_net_addr->port = [&]() -> port_type {
		if (const auto trailing = get_trailing_number(input)) {
			return static_cast<port_type>(*trailing);
		}

		return default_port;
	}();

	resolve_address_result out;

	out.host = std::move(no_port);
	out.addr = *resolved_net_addr;

	return out;
}

std::optional<netcode_address_t> get_internal_network_address() {
	const char* google_dns_server = "8.8.8.8";
	int dns_port = 53;

	struct sockaddr_in serv;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
	{
		return std::nullopt;
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(google_dns_server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));

	if (err < 0)
	{
		return std::nullopt;
	}

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*)&name, &namelen);

	if (err < 0)
	{
		return std::nullopt;
	}

	#if NETCODE_PLATFORM == NETCODE_PLATFORM_MAC || NETCODE_PLATFORM == NETCODE_PLATFORM_UNIX || PLATFORM_WEB
	close(sock);
	#elif NETCODE_PLATFORM == NETCODE_PLATFORM_WINDOWS
	closesocket(sock);
	#else
	#error unsupported platform
	#endif

	return to_netcode_addr(&name);
}

std::future<std::optional<netcode_address_t>> async_get_internal_network_address() {
	return launch_async([]() { return get_internal_network_address(); });
}

bool enable_broadcast(netcode_socket_t& socket) {
	int yes = 1;
	return setsockopt(socket.handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&yes), sizeof(yes)) == 0;
}

std::vector<netcode_address_t> get_broadcast_addresses() {
	auto out = std::vector<netcode_address_t>();

	auto add = [&](const uint32_t host_order_ip) {
		auto address = netcode_address_t();
		address.type = NETCODE_ADDRESS_IPV4;
		address.data.ipv4[0] = static_cast<uint8_t>(host_order_ip >> 24);
		address.data.ipv4[1] = static_cast<uint8_t>(host_order_ip >> 16);
		address.data.ipv4[2] = static_cast<uint8_t>(host_order_ip >> 8);
		address.data.ipv4[3] = static_cast<uint8_t>(host_order_ip);

		if (std::find(out.begin(), out.end(), address) == out.end()) {
			out.push_back(address);
		}
	};

	add(0xFFFFFFFFu);

	/*
		An interface's broadcast address: its address with all the host bits set.
	*/

	auto add_interface = [&](const uint32_t host_order_ip, const uint32_t host_order_mask) {
		const bool loopback = (host_order_ip >> 24) == 127;

		if (loopback || host_order_mask == 0xFFFFFFFFu) {
			return;
		}

		add(host_order_ip | ~host_order_mask);
	};

#if NETCODE_PLATFORM == NETCODE_PLATFORM_WINDOWS
	ULONG size = 16 * 1024;
	auto buffer = std::vector<std::byte>();

	for (int attempt = 0; attempt < 3; ++attempt) {
		buffer.resize(size);

		const auto flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
		const auto adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());
		const auto result = GetAdaptersAddresses(AF_INET, flags, nullptr, adapters, &size);

		if (result == ERROR_BUFFER_OVERFLOW) {
			continue;
		}

		if (result != NO_ERROR) {
			break;
		}

		for (auto adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
			if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
				continue;
			}

			for (auto unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next) {
				const auto sa = unicast->Address.lpSockaddr;

				if (sa == nullptr || sa->sa_family != AF_INET) {
					continue;
				}

				const auto prefix = static_cast<uint32_t>(unicast->OnLinkPrefixLength);

				if (prefix == 0 || prefix > 32) {
					continue;
				}

				const auto mask = prefix == 32 ? 0xFFFFFFFFu : ~(0xFFFFFFFFu >> prefix);
				const auto ip = ntohl(reinterpret_cast<const sockaddr_in*>(sa)->sin_addr.s_addr);

				add_interface(ip, mask);
			}
		}

		break;
	}
#elif NETCODE_PLATFORM == NETCODE_PLATFORM_MAC || NETCODE_PLATFORM == NETCODE_PLATFORM_UNIX
	ifaddrs* interfaces = nullptr;

	if (getifaddrs(&interfaces) == 0) {
		for (auto i = interfaces; i != nullptr; i = i->ifa_next) {
			if (i->ifa_addr == nullptr || i->ifa_netmask == nullptr || i->ifa_addr->sa_family != AF_INET) {
				continue;
			}

			if (!(i->ifa_flags & IFF_UP) || !(i->ifa_flags & IFF_BROADCAST) || (i->ifa_flags & IFF_LOOPBACK)) {
				continue;
			}

			const auto ip = ntohl(reinterpret_cast<const sockaddr_in*>(i->ifa_addr)->sin_addr.s_addr);
			const auto mask = ntohl(reinterpret_cast<const sockaddr_in*>(i->ifa_netmask)->sin_addr.s_addr);

			add_interface(ip, mask);
		}

		freeifaddrs(interfaces);
	}
#endif

	return out;
}
#endif

void client_send_packet_override(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<yojimbo::Client*>(context)->GetParent();
	adapter->send_packet_override(*to, reinterpret_cast<const std::byte*>(packet), bytes);
}

int client_receive_packet_override(void* context, netcode_address_t* from, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<yojimbo::Client*>(context)->GetParent();
	return adapter->receive_packet_override(*from, reinterpret_cast<std::byte*>(packet), bytes);
}

resolve_address_result client_adapter::connect(const client_connect_string& str, const client_connect_identity& identity) {
	auto resolved_addr = resolve_address_result();

#if PLATFORM_WEB
	(void)str;

	const bool use_webrtc = true;
	connected_ip_address = ::make_internal_webrtc_address(DEFAULT_GAME_PORT_V);
#else
	const bool use_webrtc = ::uses_webrtc(str);

	if (!use_webrtc) {
		host_with_default_port in;
		in.address = str;
		in.default_port = DEFAULT_GAME_PORT_V;

		resolved_addr = resolve_address(in);

		if (resolved_addr.result != resolve_result_type::OK) {
			return resolved_addr;
		}

		/*
			The game's socket is IPv4 only - an IPv6 server would only time out.
		*/

		if (resolved_addr.addr.type == NETCODE_ADDRESS_IPV6) {
			resolved_addr.result = resolve_result_type::IPV6_NOT_SUPPORTED;
			return resolved_addr;
		}
	}

	if (use_webrtc) {
		/*
			Doesnt matter, we won't use sockets either way
		*/

		connected_ip_address = ::make_internal_webrtc_address(DEFAULT_GAME_PORT_V);
	}
	else {
		connected_ip_address = resolved_addr.addr;
	}
#endif

	const auto target_addr = to_yojimbo_addr(connected_ip_address);

	auto local_addr = yojimbo::Address("127.0.0.1", DEFAULT_GAME_PORT_V);
	local_addr.SetPort(target_addr.GetPort());

	yojimbo::Address addresses[2] = {
		target_addr,
		local_addr
	};

	/*
		Over WebRTC the server's packets come from the address the data channel is given here, not the server's IP.
		The second address is the same port on localhost, e.g. for a server on this very machine behind a NAT.
	*/

	const auto num_addresses = use_webrtc ? 1 : 2;

	client.Connect(
		addresses,
		num_addresses,
		identity.expected_server_public_key.has_value() ? identity.expected_server_public_key->data() : nullptr,
		identity.session_secret_key.has_value() ? identity.session_secret_key->data() : nullptr,
		identity.blob.data()
	);

	if (auto detail = client.GetClientDetail()) {
		detail->config.auxiliary_command_function = client_auxiliary_command_function;
		detail->config.auxiliary_command_context = this;

		if (use_webrtc) {
			detail->config.override_send_and_receive = 1;
			detail->config.send_packet_override = ::client_send_packet_override;
			detail->config.receive_packet_override = ::client_receive_packet_override;
		}
	}

	return resolved_addr;
}

std::optional<augs::ed25519_public_key_type> client_adapter::get_server_public_key() const {
	if (const auto key = client.GetServerPublicKey()) {
		auto out = augs::ed25519_public_key_type();
		std::memcpy(out.data(), key, out.size());
		return out;
	}

	return std::nullopt;
}

bool client_adapter::connected_through_localhost_fallback() const {
	return client.GetServerAddressIndex() > 0;
}

double client_adapter::secs_since_last_packet() const {
	if (client.IsDisconnected()) {
		return 0.0;
	}

	if (auto* const s = client.GetClientDetail()) {
		return s->time - s->last_packet_receive_time;
	}

	return 0.0;
}

int client_adapter::get_disconnect_reason() const {
	return client.GetDisconnectReason();
}

void client_adapter::disconnect() {
	client.Disconnect();
}

void client_adapter::send_packets() {
	client.SendPackets();
}

bool client_adapter::can_send_message(const game_channel_type& channel) const {
	return client.CanSendMessage(static_cast<channel_id_type>(channel));
}

bool client_adapter::has_messages_to_send(const game_channel_type& channel) const {
	return client.HasMessagesToSend(static_cast<channel_id_type>(channel));
}

bool client_adapter::is_connected() const {
	return client.IsConnected();
}

bool client_adapter::is_connecting() const {
	return client.IsConnecting();
}

bool client_adapter::is_disconnected() const {
	return client.IsDisconnected();
}

bool client_adapter::has_connection_failed() const {
	return client.ConnectionFailed();
}

void client_adapter::set(augs::maybe_network_simulator s) {
	/*
		yojimbo only has a network simulator while connecting or connected,
		and asserts if we try to configure it otherwise.
	*/
	if (client.IsDisconnected()) {
		return;
	}

	if (!s.is_enabled) {
		s = augs::network_simulator_settings::zero();
	}

	const auto& v = s.value;

	client.SetLatency(v.latency_ms);
	client.SetJitter(v.jitter_ms);
	client.SetPacketLoss(v.loss_percent);
	client.SetDuplicates(v.duplicates_percent);
}

void server_adapter::set(augs::maybe_network_simulator s) {
	/*
		yojimbo only has a network simulator while running,
		and asserts if we try to configure it otherwise.
	*/
	if (!server.IsRunning()) {
		return;
	}

	if (!s.is_enabled) {
		s = augs::network_simulator_settings::zero();
	}

	const auto& v = s.value;

	server.SetLatency(v.latency_ms);
	server.SetJitter(v.jitter_ms);
	server.SetPacketLoss(v.loss_percent);
	server.SetDuplicates(v.duplicates_percent);
}

static auto to_network_info(const yojimbo::NetworkInfo& n) {
	network_info o;

	o.rtt_ms = n.RTT;
	o.loss_percent = n.packetLoss;
	o.sent_kbps = n.sentBandwidth;
	o.received_kbps = n.receivedBandwidth;
	o.acked_kbps = n.ackedBandwidth;
	o.packets_sent = n.numPacketsSent;
	o.packets_received = n.numPacketsReceived;
	o.packets_acked = n.numPacketsAcked;

	return o;
}

network_info client_adapter::get_network_info() const {
	yojimbo::NetworkInfo info;
	client.GetNetworkInfo(info);
	return to_network_info(info);
}

network_info server_adapter::get_network_info(const client_id_type id) const {
	yojimbo::NetworkInfo info;
	server.GetNetworkInfo(id, info);
	return to_network_info(info);
}

server_network_info server_adapter::get_server_network_info() const {
	server_network_info total;

	if (!is_running()) {
		return total;
	}

	for (client_id_type i = 0; i < static_cast<client_id_type>(max_incoming_connections_v); ++i) {
		if (!is_client_connected(i)) {
			continue;
		}

		yojimbo::NetworkInfo info;
		server.GetNetworkInfo(i, info);

		total.sent_kbps += info.sentBandwidth;
		total.received_kbps += info.receivedBandwidth;
	}

	return total;
}

bool server_adapter::send(
	const client_id_type& client_id, 
	const game_channel_type& channel_id, 
	const translated_payload_id& new_message
) {
	if (!is_valid(new_message)) {
		return false;
	}

	const auto channel_id_int = static_cast<channel_id_type>(channel_id);
	server.SendMessage(client_id, channel_id_int, new_message);

	return true;
}

std::size_t server_adapter::num_connected_clients() const {
	return server.GetNumConnectedClients();
}

void server_adapter::set_refusing_new_clients(const bool refusing) {
	if (const auto netcode_server = server.GetServerDetail()) {
		netcode_server_set_refusing_new_clients(netcode_server, refusing ? 1 : 0);
	}
}

netcode_address_t* server_adapter::get_client_address(const client_id_type& id) const {
	return server.GetClientAddress(id);
}

void server_adapter::send_udp_packet(const netcode_address_t& in_to, std::byte* const packet_data, const std::size_t packet_bytes) const {
	if (auto* const s = server.GetServerDetail()) {
		auto to = in_to;

		if (to.type == NETCODE_ADDRESS_IPV4) {
			netcode_socket_send_packet(&s->socket_holder.ipv4, &to, reinterpret_cast<void*>(packet_data), packet_bytes);
		}
		else if (to.type == NETCODE_ADDRESS_IPV6) {
			netcode_socket_send_packet(&s->socket_holder.ipv6, &to, reinterpret_cast<void*>(packet_data), packet_bytes);
		}
	}
	else {
		LOG("WARNING! Trying to send an UDP packet with a null netcode_server_t!");
	}
}

const netcode_socket_t* client_adapter::find_underlying_socket() const {
	if (client.IsDisconnected()) {
		return nullptr;
	}

	if (auto* const s = client.GetClientDetail()) {
		return std::addressof(s->socket_holder.ipv4);
	}

	return nullptr;
}

netcode_address_t client_adapter::get_connected_ip_address() const {
	return connected_ip_address;
}

const netcode_socket_t* server_adapter::find_underlying_socket() const {
	if (!server.IsRunning()) {
		return nullptr;
	}

	if (auto* const s = server.GetServerDetail()) {
		return std::addressof(s->socket_holder.ipv4);
	}

	return nullptr;
}
