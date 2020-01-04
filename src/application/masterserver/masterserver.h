#pragma once
#include "application/config_lua_table.h"
#include "game/modes/all_mode_includes.h"

enum class masterserver_udp_command : uint8_t {
	HEARTBEAT, // server to client
	NAT_OPEN_REQUEST // client to server
};

struct server_heartbeat {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_heartbeat
	server_name_type server_name;
	arena_identifier current_arena;
	mode_type_id game_mode;
	uint8_t num_players;
	uint8_t max_players;
	// END GEN INTROSPECTOR
};

void perform_masterserver(const config_lua_table&);
