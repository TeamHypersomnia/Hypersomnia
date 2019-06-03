#include "application/network/server_adapter.h"
#include "application/network/client_adapter.h"

#include "augs/network/network_types.h"
#include "augs/readwrite/memory_stream.h"
#include "hypersomnia_version.h"

#include "application/network/net_message_serialization.h"

#include "3rdparty/yojimbo/netcode.io/netcode.c"

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
	numChannels = 3;
	timeout = 10;

#if RESYNCS_CHANNEL
	{
		auto& resyncs = channel[static_cast<int>(game_channel_type::RESYNCS)];
		resyncs.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		resyncs.maxBlockSize = 1024 * 1024 * 2;
		resyncs.sentPacketBufferSize = 16;
		resyncs.messageResendTime = 0.f;
		resyncs.messageSendQueueSize = 16;
		resyncs.messageReceiveQueueSize = 16;
		resyncs.packetBudget = 600;
	}
#endif

	{
		auto& solvable_stream = channel[static_cast<int>(game_channel_type::SERVER_SOLVABLE_AND_STEPS)];
		solvable_stream.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		solvable_stream.maxBlockSize = 1024 * 1024 * 2;
		solvable_stream.sentPacketBufferSize = 1024;
		solvable_stream.messageResendTime = 0.f;
		solvable_stream.messageSendQueueSize = 1024 * 8;
		solvable_stream.messageReceiveQueueSize = 1024 * 8;
	}

	{
		auto& client_entropies = channel[static_cast<int>(game_channel_type::CLIENT_COMMANDS)];
		client_entropies.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		/* these are like, super critical. */
		client_entropies.sentPacketBufferSize = 1024;
		client_entropies.messageResendTime = 0.f;
		client_entropies.messageSendQueueSize = 1024 * 8;
		client_entropies.messageReceiveQueueSize = 1024 * 8;
	}

	{
		auto& communications = channel[static_cast<int>(game_channel_type::COMMUNICATIONS)];
		communications.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
	}

	serverPerClientMemory += 1024 * 1024 * 7;
	clientMemory = 1024 * 1024 * 50;

	networkSimulator = true;

	set_max_packet_size(2 * 1024);
}

void game_connection_config::set_max_packet_size(const unsigned s) {
	protocolId = 8412;

	maxPacketSize = s;
    maxPacketFragments = (int) ceil( maxPacketSize / packetFragmentSize );
}

game_connection_config::game_connection_config(const server_start_input& in) : game_connection_config() {
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

server_adapter::server_adapter(const server_start_input& in) :
	connection_config(in),
	adapter(this),
	server(
		yojimbo::GetDefaultAllocator(), 
		privateKey.data(), 
		yojimbo::Address(in.ip.c_str(), in.port), 
		connection_config, 
		adapter, 
		yojimbo_time()
	)
{
    server.Start(in.max_connections);

	const auto addr = server.GetAddress();

    char buffer[256];
    addr.ToString(buffer, sizeof(buffer));

	LOG("Server address is %x", buffer);
}

void server_adapter::disconnect_client(const client_id_type& id) {
	server.DisconnectClient(id);
}

void server_adapter::send_packets() {
	server.SendPackets();
}

client_adapter::client_adapter() :
	connection_config(),
	adapter(nullptr),
	client(
		yojimbo::GetDefaultAllocator(), 
		yojimbo::Address("0.0.0.0"), 
		connection_config, 
		adapter, 
		yojimbo_time()
	)
{}

void client_adapter::connect(const client_start_input& in) {
	uint64_t clientId;
	yojimbo::random_bytes((uint8_t*)&clientId, 8);

	const auto target_addr = yojimbo::Address(in.ip_port.c_str());

	auto local_addr = yojimbo::Address("127.0.0.1", 8412);

	if (target_addr.GetType() == yojimbo::AddressType::ADDRESS_IPV6) {
		local_addr = yojimbo::Address("::1", 8412);
	}

	local_addr.SetPort(target_addr.GetPort());

	yojimbo::Address addrs[2] = {
		target_addr,
		local_addr
	};

	client.InsecureConnect(
		privateKey.data(), 
		clientId,
		addrs,
		2
	);
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
	const auto s = server.GetServerDetail();
	auto addr = s->client_address[id];

	char buffer[NETCODE_MAX_ADDRESS_STRING_LENGTH];
	netcode_address_to_string(&addr, buffer);
	
	return yojimbo::Address(buffer);
}

yojimbo::Address client_adapter::get_server_address() const {
	return client.GetAddress();
}
