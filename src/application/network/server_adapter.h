#pragma once
#include <functional>
#include "augs/global_libraries.h"
#include "application/network/network_adapters.h"

struct netcode_socket_t;

struct netcode_address_t;

using auxiliary_command_callback_type = std::function<bool (const netcode_address_t&, const std::byte*, std::size_t n)>;
using send_packet_override_type = std::function<bool (const netcode_address_t&,const std::byte*,int)>;
using receive_packet_override_type = std::function<int (netcode_address_t&,std::byte*,int)>;

class server_adapter {
	friend bool auxiliary_command_function(void* context, struct netcode_address_t* from, uint8_t* packet, int bytes);
	friend void send_packet_override(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes);
	friend bool aux_send_packet(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes);
	friend int receive_packet_override(void* context, netcode_address_t* from, uint8_t* packet, int bytes);

	std::array<uint8_t, yojimbo::KeyBytes> privateKey = {};
	game_connection_config connection_config;
	GameAdapter adapter;
	yojimbo::DefaultAllocator yojimbo_allocator;
	yojimbo::Server server;
	auxiliary_command_callback_type auxiliary_command_callback;
	send_packet_override_type send_packet_override;
	receive_packet_override_type receive_packet_override;

	struct connection_event {
		client_id_type client_id = dead_client_id_v;
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
	server_adapter(
		const augs::server_listen_input&,
		bool is_integrated,
		auxiliary_command_callback_type,
		send_packet_override_type,
		receive_packet_override_type,
		bool is_webrtc_only
	);

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

	netcode_address_t* get_client_address(const client_id_type& id) const;

	void send_udp_packet(const netcode_address_t& to, std::byte*, std::size_t n) const;
	const netcode_socket_t* find_underlying_socket() const;
};
