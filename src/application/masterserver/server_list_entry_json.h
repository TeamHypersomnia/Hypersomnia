#pragma once
#include "application/masterserver/server_heartbeat.h"

struct server_list_entry_json {
	// GEN INTROSPECTOR struct server_list_entry_json
	std::string server_version;

	std::string name;
	std::string ip;

	double time_hosted = 0.0;
	double time_last_heartbeat = 0.0;

	std::string arena;
	std::string game_mode;

	uint8_t num_playing = 0;
	uint8_t num_spectating = 0;
	uint8_t slots = 0;

	nat_type nat = nat_type::PUBLIC_INTERNET;

	std::optional<std::string> internal_network_address;
	std::optional<bool> is_editor_playtesting_server;

	uint8_t score_resistance = 0;
	uint8_t score_metropolis = 0;

	augs::constant_size_vector<server_heartbeat_player_info, 32> players_resistance;
	augs::constant_size_vector<server_heartbeat_player_info, 32> players_metropolis;
	augs::constant_size_vector<server_heartbeat_player_info, 32> players_spectating;
	// END GEN INTROSPECTOR
};


