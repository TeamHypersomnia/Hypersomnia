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

constexpr std::size_t chosen_packet_size_v = 1024;

constexpr std::size_t total_header_bytes_v = 
	yojimbo::ConservativeMessageHeaderBits
	+ yojimbo::ConservativePacketHeaderBits 
	+ yojimbo::ConservativeMessageHeaderBits 
;

/* Make the max message size conservative enough so that a single message does not cause packets to split. */
constexpr std::size_t max_message_size_v = ((chosen_packet_size_v - total_header_bytes_v) / 4) * 4;

#include "application/network/network_serialization.h"

enum class message_handler_result {
	ABORT_AND_DISCONNECT,
	CONTINUE
};

enum class connection_event_type {
	CONNECTED,
	DISCONNECTED
};

struct connection_event {
	client_id_type subject_id = -1;
	connection_event_type type = connection_event_type::CONNECTED;
};

#define NULL_SERIALIZER() \
	template <typename Stream> \
	bool Serialize(Stream& stream) { \
		(void)stream; \
		return true; \
	}

struct preserialized_message : public yojimbo::Message {
	message_bytes_type bytes;

	template <typename Stream>
	bool Serialize(Stream& stream) {
        int length = 0;

        if ( Stream::IsWriting ) {
			length = (int) bytes.size();
        }

        serialize_int( stream, length, 0, max_message_size_v );
		serialize_bytes( stream, (uint8_t*)bytes.data(), length );

		return true;
	}

	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct only_block_message : public yojimbo::BlockMessage {
	/* The client will never send blocks */

	static constexpr bool server_to_client = true;
	static constexpr bool client_to_server = false;

	NULL_SERIALIZER();
	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

namespace net_messages {
	struct initial_solvable : only_block_message {};
	struct initial_steps_correction : only_block_message {};

	struct step_entropy : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct client_entropy : preserialized_message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct client_welcome : public yojimbo::Message, public requested_client_settings {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_string(stream, chosen_nickname, buf_len);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	using all_t = type_list<
		initial_solvable*,
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
	GameConnectionConfig(const server_start_input&);

	void set_max_packet_size(unsigned);
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
	message_handler_result process_message(const client_id_type& id, yojimbo::Message&, F&& message_callback);

public:
	server_adapter(const server_start_input&);

	template <class F>
	void advance(
		const net_time_t server_time, 
		F&& message_callback
	);

	void send_packets();

	bool is_running() const;
	bool can_send_message(const client_id_type&, const game_channel_type&) const;
	bool has_messages_to_send(const client_id_type&, const game_channel_type&) const;

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
