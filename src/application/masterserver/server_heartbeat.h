#pragma once
#include "game/modes/all_mode_includes.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"

struct server_heartbeat {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_heartbeat
	server_name_type server_name;
	arena_identifier current_arena;
	online_mode_type_id game_mode;

	uint8_t num_fighting;
	uint8_t max_fighting;

	uint8_t num_online;
	uint8_t max_online;

	netcode_address_t internal_network_address;
	// END GEN INTROSPECTOR
};
