#pragma once
#include "3rdparty/yojimbo/yojimbo.h"
#include "application/setups/server/server_start_input.h"
#include "augs/misc/timing/timer.h"
#include "game/modes/mode_entropy.h"

using client_id_type = int;

enum class GameMessageType {
	INITIAL_SOLVABLE,
	STEP_ENTROPY,
	CLIENT_ENTROPY,

	COUNT
};

namespace net_messages {
	struct initial_solvable : public yojimbo::BlockMessage {
		std::vector<std::byte> bytes;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct step_entropy : public yojimbo::Message {
		mode_entropy entropy;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct client_entropy : public yojimbo::Message {
		total_mode_player_entropy entropy;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			(void)stream;
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};
}

enum class GameChannel {
	SOLVABLE_STREAM,
	COMMUNICATIONS,

	COUNT
};

struct GameConnectionConfig : yojimbo::ClientServerConfig {
	GameConnectionConfig();
};

class server_adapter;

// the message factory
YOJIMBO_MESSAGE_FACTORY_START(GameMessageFactory, (int)GameMessageType::COUNT);
YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::INITIAL_SOLVABLE, net_messages::initial_solvable);
YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::STEP_ENTROPY, net_messages::step_entropy);
YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::CLIENT_ENTROPY, net_messages::client_entropy);
YOJIMBO_MESSAGE_FACTORY_FINISH();

class GameAdapter : public yojimbo::Adapter {
public:
    explicit GameAdapter(server_adapter* const server = nullptr) :
        m_server(server) {}

    yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) override {
        return YOJIMBO_NEW(allocator, GameMessageFactory, allocator);
    }

	void OnServerClientConnected(const client_id_type clientIndex) override;
	void OnServerClientDisconnected(const client_id_type clientIndex) override;
   
private:
    server_adapter* m_server;
};

class server_adapter {
	std::array<uint8_t, yojimbo::KeyBytes> privateKey = {};
	GameConnectionConfig connection_config;
	GameAdapter adapter;
	yojimbo::Server server;

	friend GameAdapter;

	void client_connected(const client_id_type id);
	void client_disconnected(const client_id_type id);

	template <class F>
	void process_message(client_id_type id, yojimbo::Message&, F&& message_callback);

public:
	server_adapter(const server_start_input&);

	template <class F>
	void advance(
		const double server_time, 
		F&& message_callback
	);

	bool is_running() const;

	void disconnect_client(const client_id_type id);
	void deal_with_malicious_client(const client_id_type id);

	std::string describe_client(const client_id_type id) const;

	auto& get_specific() {
		return server;
	}

	const auto& get_specific() const {
		return server;
	}
};
