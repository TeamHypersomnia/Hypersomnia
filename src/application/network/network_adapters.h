#pragma once
#include "3rdparty/yojimbo/yojimbo.h"
#undef write_bytes
#undef read_bytes

#include "application/setups/server/server_start_input.h"
#include "augs/misc/timing/timer.h"
#include "game/modes/mode_entropy.h"
#include "augs/network/network_types.h"
#include "augs/enums/callback_result.h"
#include "augs/ensure.h"

#include "application/network/requested_client_settings.h"

#include "application/network/network_messages.h"
#include "application/network/custom_yojimbo_factory.h"

enum class game_channel_type {
	SOLVABLE_STREAM,
	COMMUNICATIONS,

	COUNT
};

struct game_connection_config : yojimbo::ClientServerConfig {
	game_connection_config();
	game_connection_config(const server_start_input&);

	void set_max_packet_size(unsigned);
};

class server_adapter;

class GameAdapter : public yojimbo::Adapter {
public:
    explicit GameAdapter(server_adapter* const server = nullptr) :
        m_server(server) {}

    yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) override {
        return YOJIMBO_NEW(allocator, GameMessageFactory, allocator);
    }

	void OnServerClientConnected(client_id_type clientIndex) override;
	void OnServerClientDisconnected(client_id_type clientIndex) override;
   
private:
    server_adapter* m_server;
};

class server_adapter {
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
		return server;
	}

	const auto& get_specific() const {
		return server;
	}
};
