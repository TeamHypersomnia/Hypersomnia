#include "application/network/server_adapter.h"
#include "application/network/client_adapter.h"

#include "augs/network/network_types.h"
#include "augs/readwrite/memory_stream.h"
#include "hypersomnia_version.h"

#include "application/network/net_message_serialization.h"

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

void server_adapter::client_connected(const client_id_type id) {
	pending_events.push_back({ id, true });
}

void server_adapter::client_disconnected(const client_id_type id) {
	pending_events.push_back({ id, false });
}

game_connection_config::game_connection_config() {
	numChannels = 3;

	{
		auto& solvable_stream = channel[static_cast<int>(game_channel_type::SERVER_SOLVABLE_AND_STEPS)];
		solvable_stream.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		solvable_stream.maxBlockSize = 1024 * 1024 * 2;
	}

	{
		auto& client_entropies = channel[static_cast<int>(game_channel_type::CLIENT_ENTROPIES)];
		client_entropies.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		/* these are like, super critical. */
		client_entropies.messageResendTime = 0.f;
	}

	{
		auto& communications = channel[static_cast<int>(game_channel_type::COMMUNICATIONS)];
		communications.type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
	}

	serverPerClientMemory += 1024 * 1024 * 2;

	set_max_packet_size(2 * 1024);
}

void game_connection_config::set_max_packet_size(const unsigned s) {
	protocolId = hypersomnia_version().commit_number;

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

    if (!is_running()) {
        throw std::runtime_error("Could not start server at port " + std::to_string(addr.GetPort()));
    }

    char buffer[256];
    addr.ToString(buffer, sizeof(buffer));

	LOG("Server address is %x", buffer);
}

void server_adapter::disconnect_client(const client_id_type& id) {
	(void)id;
	//server.DisconnectClient(id);
}

void server_adapter::send_packets() {
	//server.SendPackets();
}

client_adapter::client_adapter(const client_start_input& in) :
	connection_config(),
	adapter(nullptr),
	client(
		yojimbo::GetDefaultAllocator(), 
		yojimbo::Address(in.ip_port.c_str()), 
		connection_config, 
		adapter, 
		yojimbo_time()
	)
{

}
