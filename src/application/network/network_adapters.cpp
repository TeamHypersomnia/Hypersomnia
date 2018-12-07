#include "application/network/network_adapters.h"
#include "augs/network/network_types.h"

static_assert(max_incoming_connections_v == yojimbo::MaxClients);

void server_adapter::client_connected(const client_id_type id) {
	pending_events.push_back({ id, connection_event_type::CONNECTED });
}

void server_adapter::client_disconnected(const client_id_type id) {
	pending_events.push_back({ id, connection_event_type::DISCONNECTED });
}

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

GameConnectionConfig::GameConnectionConfig() {
	numChannels = 1;
	channel[static_cast<int>(game_channel_type::SOLVABLE_STREAM)].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
	set_max_packet_size(2 * 1024);
}

void GameConnectionConfig::set_max_packet_size(const unsigned s) {
	maxPacketSize = s;
    maxPacketFragments = (int) ceil( maxPacketSize / packetFragmentSize );
}

GameConnectionConfig::GameConnectionConfig(const server_start_input& in) : GameConnectionConfig() {
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
		0.0
	)
{
    server.Start(in.max_connections);

	const auto addr = server.GetAddress();

    if (!is_running()) {
        throw std::runtime_error("Could not start server at port " + std::to_string(addr.GetPort()));
    }

    char buffer[256];
    addr.ToString(buffer, sizeof(buffer));

	LOG("Server address is %x", buffer);
}

void server_adapter::disconnect_client(const client_id_type& id) {
	server.DisconnectClient(id);
}


