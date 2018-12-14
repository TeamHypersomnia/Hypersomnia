#pragma once
#include "application/network/network_adapters.h"

class client_adapter {
	std::array<uint8_t, yojimbo::KeyBytes> privateKey = {};
	game_connection_config connection_config;
	GameAdapter adapter;
	yojimbo::Client client;

	friend GameAdapter;

	template <class H>
	message_handler_result process_message(yojimbo::Message&, H&& handler);

	template <class T>
	auto create_message(const client_id_type&);

public:
	client_adapter(const client_start_input&);

	template <class H>
	void advance(
		const net_time_t client_time, 
		H&& handler
	);

	void send_packets();

	bool is_running() const;
	bool can_send_message(const client_id_type&, const game_channel_type&) const;
	bool has_messages_to_send(const client_id_type&, const game_channel_type&) const;

	template <class... Args>
	bool send_payload(
		const client_id_type& client_id, 
		const game_channel_type& channel_id, 
		Args&&... args
	);

	bool is_client_connected(const client_id_type& id) const;

	void disconnect_client(const client_id_type& id);

	auto& get_specific() {
		return client;
	}

	const auto& get_specific() const {
		return client;
	}
};
