#pragma once
#include "augs/global_libraries.h"
#include "application/network/network_adapters.h"

class server_adapter {
	augs::network_raii raii;

	std::array<uint8_t, yojimbo::KeyBytes> privateKey = {};
	game_connection_config connection_config;
	GameAdapter adapter;
	yojimbo::Server server;

	struct connection_event {
		client_id_type client_id = -1;
		bool connected = false;
	};

	std::vector<connection_event> pending_events;

	friend GameAdapter;

	void client_connected(client_id_type id);
	void client_disconnected(client_id_type id);

	template <class H>
	void process_connections_disconnections(H&& handler);

	template <class H>
	message_handler_result process_message(const client_id_type& id, yojimbo::Message&, H&& handler);

	template <class T>
	auto create_message(const client_id_type&);

public:
	server_adapter(const server_start_input&);

	template <class H>
	void advance(
		const net_time_t server_time, 
		H&& handler
	);

	void send_packets();
	void stop();

	bool is_running() const;
	bool can_send_message(const client_id_type&, const game_channel_type&) const;
	bool has_messages_to_send(const client_id_type&, const game_channel_type&) const;

	template <class... Args>
	bool send_payload(
		const client_id_type& client_id, 
		const game_channel_type& channel_id, 
		Args&&... args
	);

	template <class... Args>
	translated_payload_id translate_payload(
		const client_id_type& client_id, 
		Args&&... args
	);

	bool send(
		const client_id_type& client_id, 
		const game_channel_type& channel_id, 
		const translated_payload_id&
	);

	bool is_client_connected(const client_id_type& id) const;

	void disconnect_client(const client_id_type& id);

	auto& get_specific() {
		return server;
	}

	const auto& get_specific() const {
		return server;
	}

	void set(augs::maybe_network_simulator);

	network_info get_network_info(client_id_type) const;
	server_network_info get_server_network_info() const;

	std::size_t num_connected_clients() const;

	yojimbo::Address get_client_address(const client_id_type& id) const;
};
