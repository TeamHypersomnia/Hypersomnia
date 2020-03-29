#pragma once
#include "game/modes/all_mode_includes.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/nat/nat_type_detector.h"

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

	std::optional<netcode_address_t> internal_network_address;
	nat_type nat = nat_type::PUBLIC_INTERNET;
	// END GEN INTROSPECTOR

	void validate();

	bool is_behind_nat() const {
		return nat != nat_type::PUBLIC_INTERNET;
	}

	int get_num_spectators() const {
		return num_online - num_fighting;
	}

	int get_max_spectators() const {
		return max_online - num_fighting;
	}
};
