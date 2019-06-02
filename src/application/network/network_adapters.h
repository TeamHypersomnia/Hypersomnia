#pragma once
#include "3rdparty/yojimbo/yojimbo.h"
#undef write_bytes
#undef read_bytes

#include "application/setups/server/server_start_input.h"
#include "application/setups/client/client_start_input.h"

#include "augs/misc/timing/timer.h"
#include "game/modes/mode_entropy.h"
#include "augs/network/network_types.h"
#include "augs/enums/callback_result.h"
#include "augs/ensure.h"

#include "application/network/requested_client_settings.h"

#include "application/network/network_messages.h"
#include "application/network/custom_yojimbo_factory.h"

enum class game_channel_type {
#if RESYNCS_CHANNEL
	RESYNCS,
#endif
	SERVER_SOLVABLE_AND_STEPS,
	CLIENT_COMMANDS,
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

using translated_payload_id = yojimbo::Message*;

inline bool is_valid(const translated_payload_id& t) {
	return t != nullptr;
}
