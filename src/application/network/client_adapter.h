#pragma once
#include <cstddef>
#include <cstdint>
#include "augs/global_libraries.h"
#include "application/network/network_adapters.h"
#include "augs/network/network_simulator_settings.h"
#include "application/network/resolve_address_result.h"
#include "application/network/game_channel_type.h"
#include "augs/misc/ed25519.h"
#include "application/network/identity_cert.h"

struct netcode_socket_t;

using client_auxiliary_command_callback_type = std::function<bool (std::byte*, std::size_t n)>;
using send_packet_override_type = std::function<bool (const netcode_address_t&,const std::byte*,int)>;
using receive_packet_override_type = std::function<int (netcode_address_t&,std::byte*,int)>;

/*
	What a client brings to its handshake - see identity_cert.h.
*/

struct client_connect_identity {
	/*
		The server key from the server list, or none to take the one the server presents (connecting by address).
	*/

	std::optional<augs::ed25519_public_key_type> expected_server_public_key;

	/*
		The session secret key (seed and public key) that signs the connection, if there is a cert to go with it.
	*/

	std::optional<std::array<uint8_t, 64>> session_secret_key;

	identity_cert::blob_type blob = {};
};

class client_adapter {
	friend bool client_auxiliary_command_function(void* context, uint8_t* packet, int bytes);
	friend void client_send_packet_override(void* context, netcode_address_t* to, NETCODE_CONST uint8_t* packet, int bytes);
	friend int client_receive_packet_override(void* context, netcode_address_t* from, uint8_t* packet, int bytes);

	game_connection_config connection_config;
	GameAdapter adapter;
	yojimbo::DefaultAllocator yojimbo_allocator;
	yojimbo::Client client;
	client_auxiliary_command_callback_type auxiliary_command_callback;
	send_packet_override_type send_packet_override;
	receive_packet_override_type receive_packet_override;
	netcode_address_t connected_ip_address;

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
	client_adapter(
		std::optional<port_type> preferred_binding_port,
		client_auxiliary_command_callback_type,
		send_packet_override_type,
		receive_packet_override_type
	);

	auto& get_allocator() {
		return yojimbo_allocator;
	}

	resolve_address_result connect(const client_connect_string&, const client_connect_identity&);

	/*
		The key the server presented, once it got that far.
	*/

	std::optional<augs::ed25519_public_key_type> get_server_public_key() const;
	bool connected_through_localhost_fallback() const;

	/*
		yojimbo's reason for the last failed or ended connection (YOJIMBO_CLIENT_DISCONNECT_REASON_*).
	*/

	int get_disconnect_reason() const;

	/*
		Since the last packet from the server, by netcode's clock - 0 when not connected.
	*/

	double secs_since_last_packet() const;

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

	netcode_address_t get_connected_ip_address() const;
};
