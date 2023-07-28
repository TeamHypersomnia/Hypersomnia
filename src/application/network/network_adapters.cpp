#include "application/network/resolve_address.h"
#include "application/network/server_adapter.h"
#include "application/network/client_adapter.h"

#include "augs/network/network_types.h"
#include "augs/readwrite/memory_stream.h"
#include "hypersomnia_version.h"

#include "application/network/net_serialize.h"
#include "3rdparty/yojimbo/netcode.io/netcode.c"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/templates/thread_templates.h"

#undef SendMessage
#undef SetPort

static_assert(max_incoming_connections_v == yojimbo::MaxClients);

void GameAdapter::OnServerClientConnected(const client_id_type clientIndex) {
	if (m_server != nullptr) {
		m_server->client_connected(clientIndex);
	}
}

void GameAdapter::OnServerClientDisconnected(const client_id_type clientIndex) {
	if (m_server != nullptr) {
		m_server->client_disconnected(clientIndex);
	}
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
	timeout = 10;

	{
		auto& solvable_stream = channel[static_cast<int>(game_channel_type::RELIABLE_MESSAGES)];
		solvable_stream.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		solvable_stream.maxBlockSize = max_block_size_v;
		solvable_stream.sentPacketBufferSize = 1024 * 4;
		solvable_stream.maxMessagesPerPacket = 32;
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
	protocolId = 8412;

	maxPacketSize = s;
    maxPacketFragments = (int) ceil( maxPacketSize / packetFragmentSize );
}

game_connection_config::game_connection_config(const augs::server_listen_input& in) : game_connection_config() {
	(void)in;
#if 0
	set_max_packet_size(in.max_packet_size);
#endif
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
		default:
			return typesafe_sprintf("Couldn't resolve %x: unknown error.", host);
	}
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
	return try_fire_interval(interval, when_last, yojimbo_time());
}

bool auxiliary_command_function(void* context, struct netcode_address_t* from, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<server_adapter*>(context);
	return adapter->auxiliary_command_callback(*from, reinterpret_cast<const std::byte*>(packet), bytes);
}

server_adapter::server_adapter(
	const augs::server_listen_input& in, 
	const bool is_integrated, 
	auxiliary_command_callback_type auxiliary_command_callback
) :
	connection_config(in),
	adapter(this),
	server(
		yojimbo::GetDefaultAllocator(), 
		privateKey.data(), 
		yojimbo::Address(in.ip.c_str(), in.port), 
		connection_config, 
		adapter, 
		yojimbo_time()
	),
	auxiliary_command_callback(auxiliary_command_callback)
{
	auto slots = in.slots;

	if (is_integrated) {
		slots -= 1;
	}

	server.Start(std::max(1, slots));
	LOG("Server address is %x", ToString(server.GetAddress()));

	if (auto detail = server.GetServerDetail()) {
		detail->config.auxiliary_command_function = auxiliary_command_function;
		detail->config.auxiliary_command_context = this;
	}
}

void server_adapter::disconnect_client(const client_id_type& id) {
	server.DisconnectClient(id);
}

void server_adapter::send_packets() {
	server.SendPackets();
}

void server_adapter::send_packets_to(const client_id_type& id) {
	server.SendPacketsTo(id);
}

void server_adapter::receive_packets_from(const client_id_type& id) {
	server.ReceivePacketsFrom(id);
}

bool client_auxiliary_command_function(void* context, uint8_t* packet, int bytes) {
	auto* adapter = reinterpret_cast<client_adapter*>(context);
	return adapter->auxiliary_command_callback(reinterpret_cast<std::byte*>(packet), bytes);
}

client_adapter::client_adapter(
	const std::optional<port_type> preferred_binding_port,
	client_auxiliary_command_callback_type auxiliary_command_callback
) :
	connection_config(),
	adapter(nullptr),
	client(
		yojimbo::GetDefaultAllocator(), 
		preferred_binding_port ? yojimbo::Address("0.0.0.0", *preferred_binding_port) : yojimbo::Address("0.0.0.0"), 
		connection_config, 
		adapter, 
		yojimbo_time()
	),
	auxiliary_command_callback(auxiliary_command_callback)
{
}

std::optional<unsigned long> get_trailing_number(const std::string& s);
std::string cut_trailing_number(const std::string& s);

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

std::optional<netcode_address_t> to_netcode_addr(const std::string& ip, const port_type port) {
	struct netcode_address_t addr;

	if (NETCODE_OK == netcode_parse_address(ip.c_str(), &addr)) {
		addr.port = port;
		return addr;
	}

	return std::nullopt;
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

std::future<resolve_address_result> async_resolve_address(const address_and_port& in) {
	return launch_async([in]() { return resolve_address(in); });
}

std::optional<netcode_address_t> to_netcode_addr(const address_and_port& in) {
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

resolve_address_result resolve_address(const address_and_port& in) {
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

	#if NETCODE_PLATFORM == NETCODE_PLATFORM_MAC || NETCODE_PLATFORM == NETCODE_PLATFORM_UNIX
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

resolve_address_result client_adapter::connect(const address_and_port& in) {
	uint64_t clientId;
	yojimbo::random_bytes((uint8_t*)&clientId, 8);

	const auto resolved_addr = resolve_address(in);

	if (resolved_addr.result != resolve_result_type::OK) {
		return resolved_addr;
	}

	const auto& target_addr = to_yojimbo_addr(resolved_addr.addr);

	auto local_addr = yojimbo::Address("127.0.0.1", 8412);

	if (target_addr.GetType() == yojimbo::AddressType::ADDRESS_IPV6) {
		local_addr = yojimbo::Address("::1", 8412);
	}

	local_addr.SetPort(target_addr.GetPort());

	yojimbo::Address addresses[2] = {
		target_addr,
		local_addr
	};

	client.InsecureConnect(
		privateKey.data(), 
		clientId,
		addresses,
		2
	);

	if (auto detail = client.GetClientDetail()) {
		detail->config.auxiliary_command_function = client_auxiliary_command_function;
		detail->config.auxiliary_command_context = this;
	}

	return resolved_addr;
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

yojimbo::Address server_adapter::get_client_address(const client_id_type& id) const {
	if (const auto s = server.GetServerDetail()) {
		auto addr = s->client_address[id];

		char buffer[NETCODE_MAX_ADDRESS_STRING_LENGTH];
		netcode_address_to_string(&addr, buffer);

		return yojimbo::Address(buffer);
	}
	else {
		LOG("WARNING! Trying to get_client_address with a null netcode_server_t!");
		return yojimbo::Address();
	}
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

yojimbo::BlockProgress client_adapter::get_block_progress(const game_channel_type channel) const {
	return client.GetBlockProgress(static_cast<int>(channel));
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
