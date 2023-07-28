#pragma once
#include "augs/global_libraries.h"
#include "application/network/network_adapters.h"
#include "augs/network/network_simulator_settings.h"
#include "application/network/game_channel_type.h"

struct netcode_socket_t;
struct resolve_address_result;

using client_auxiliary_command_callback_type = std::function<bool (std::byte*, std::size_t n)>;

class client_adapter {
	friend bool client_auxiliary_command_function(void* context, uint8_t* packet, int bytes);

	std::array<uint8_t, yojimbo::KeyBytes> privateKey = {};
	game_connection_config connection_config;
	GameAdapter adapter;
	yojimbo::Client client;
	client_auxiliary_command_callback_type auxiliary_command_callback;

	friend GameAdapter;

	template <class H>
	message_handler_result process_message(yojimbo::Message&, H&& handler);

	template <class T>
	auto create_message();

	auto& get_specific() {
		return client;
	}

	const auto& get_specific() const {
		return client;
	}

	bool can_send_message(const game_channel_type&) const;
	bool has_messages_to_send(const game_channel_type&) const;

public:
	client_adapter(std::optional<port_type> preferred_binding_port, client_auxiliary_command_callback_type);
	resolve_address_result connect(const address_and_port&);

	template <class H>
	void advance(
		const net_time_t client_time, 
		H&& handler
	);

	void send_packets();

	template <class... Args>
	bool send_payload(
		const game_channel_type& channel_id, 
		Args&&... args
	);

	bool is_disconnected() const;
	bool is_connected() const;
	bool is_connecting() const;
	bool has_connection_failed() const;

	void disconnect();

	void set(augs::maybe_network_simulator);
	network_info get_network_info() const;

	const netcode_socket_t* find_underlying_socket() const;

	yojimbo::BlockProgress get_block_progress(game_channel_type) const;
};
