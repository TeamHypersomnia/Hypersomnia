#include "application/network/network_adapters.h"

void server_adapter::client_connected(const client_id_type id) {
	/* TODO: research the order */
	(void)id;
}

void server_adapter::client_disconnected(const client_id_type id) {
	/* TODO: research the order */
	(void)id;
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
	channel[static_cast<int>(GameChannel::SOLVABLE_STREAM)].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
}

bool server_adapter::is_running() const {
	return server.IsRunning();
}

server_adapter::server_adapter(const server_start_input& in) :
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

std::string server_adapter::describe_client(const client_id_type id) const {
	return typesafe_sprintf(
		"Id: %x\nNickname: %x",
		id,
		"not implemented"
	);
}

void server_adapter::disconnect_client(const client_id_type id) {
	server.DisconnectClient(id);
}

void server_adapter::deal_with_malicious_client(const client_id_type id) {
	LOG("Malicious client detected. Details:\n%x");

	disconnect_client(id);
}

