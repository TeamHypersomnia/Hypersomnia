#pragma once
#include "game/modes/all_mode_includes.h"
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/nat/nat_type.h"
#include "hypersomnia_version.h"

bool operator==(const netcode_address_t& a, const netcode_address_t& b);
bool operator!=(const netcode_address_t& a, const netcode_address_t& b);

struct server_heartbeat_player_info {
	// GEN INTROSPECTOR struct server_heartbeat_player_info
	client_nickname_type nickname;
	uint8_t score = 0;
	uint8_t deaths = 0;
	// END GEN INTROSPECTOR

	bool operator<(const server_heartbeat_player_info& b) const {
		if (score == b.score && nickname == b.nickname) {
			return deaths > b.deaths;
		}

		return std::tie(score, nickname) < std::tie(b.score, b.nickname);
	}

	bool operator==(const server_heartbeat_player_info&) const = default;
};

struct server_heartbeat {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_heartbeat
	server_name_type server_name;
	arena_identifier current_arena;
	game_mode_name_type game_mode;

	uint8_t num_online_humans;
	uint8_t num_online;
	uint8_t max_online;

	std::optional<netcode_address_t> internal_network_address;
	nat_detection_result nat;
	bool suppress_new_community_server_webhook = false;
	bool show_on_server_list = true;
	game_version_identifier server_version = "Unknown";
	bool is_editor_playtesting_server = false;

	uint8_t score_resistance = 0;
	uint8_t score_metropolis = 0;

	augs::constant_size_vector<server_heartbeat_player_info, 32> players_resistance;
	augs::constant_size_vector<server_heartbeat_player_info, 32> players_metropolis;
	augs::constant_size_vector<server_heartbeat_player_info, 32> players_spectating;

	bool require_authentication = false;
	uint8_t ranked_state = 0;
	// END GEN INTROSPECTOR

	double cached_time_to_event = 0.0;

	bool is_ranked_server() const {
		return ranked_state >= 1;
	}

	bool is_still_joinable_ranked() const {
		return ranked_state == 1;
	}

	bool operator==(const server_heartbeat&) const = default;

	void validate();
	bool is_valid() const;

	bool is_full() const;

	bool is_behind_nat() const {
		return nat.type != nat_type::PUBLIC_INTERNET;
	}

	int get_num_spectators() const {
		return players_spectating.size();
	}

	std::string get_location_id() const;
};
