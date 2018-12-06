#pragma once
#include "3rdparty/yojimbo/yojimbo.h"
#include "application/setups/server/server_start_input.h"
#include "augs/misc/timing/timer.h"
#include "game/modes/mode_entropy.h"
#include "augs/network/network_types.h"
#include "augs/enums/callback_result.h"
#include "augs/ensure.h"

enum class connection_event_type {
	CONNECTED,
	DISCONNECTED
};

struct connection_event {
	client_id_type subject_id = -1;
	connection_event_type type = connection_event_type::CONNECTED;
};

namespace net_messages {
	struct initial_solvable : public yojimbo::BlockMessage {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		std::vector<std::byte> bytes;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct initial_steps : public yojimbo::BlockMessage {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		std::vector<std::byte> bytes;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}
		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();

	};

	struct initial_steps_correction : public yojimbo::BlockMessage {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		std::vector<std::byte> bytes;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct step_entropy : public yojimbo::Message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		mode_entropy entropy;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct client_welcome : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		std::string chosen_nickname;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct client_entropy : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		total_mode_player_entropy entropy;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	using all_t = type_list<
		initial_solvable*,
		initial_steps*,
		initial_steps_correction*,
		step_entropy*,
		client_welcome*,
		client_entropy*
	>;
	
	using id_t = type_in_list_id<all_t>;
}

enum class game_channel_type {
	SOLVABLE_STREAM,
	COMMUNICATIONS,

	COUNT
};

struct GameConnectionConfig : yojimbo::ClientServerConfig {
	GameConnectionConfig();
};

class server_adapter;

#include "application/network/custom_yojimbo_factory.h"

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
	GameConnectionConfig connection_config;
	GameAdapter adapter;
	yojimbo::Server server;

	std::vector<connection_event> pending_events;

	friend GameAdapter;

	void client_connected(client_id_type id);
	void client_disconnected(client_id_type id);

	template <class F>
	void process_connections_disconnections(F&& message_callback);

	template <class F>
	callback_result process_message(const client_id_type& id, yojimbo::Message&, F&& message_callback);

public:
	server_adapter(const server_start_input&);

	template <class F>
	void advance(
		const net_time_t server_time, 
		F&& message_callback
	);

	bool is_running() const;
	bool can_send_message(const client_id_type&, const game_channel_type&) const;

	template <class T>
	void send_message(const client_id_type& client_id, const game_channel_type& channel_id, T&& message_setter);

	bool is_client_connected(const client_id_type& id) const;

	void disconnect_client(const client_id_type& id);

	auto& get_specific() {
		return server;
	}

	const auto& get_specific() const {
		return server;
	}
};
