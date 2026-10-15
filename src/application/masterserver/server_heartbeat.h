#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "game/modes/all_mode_includes.h"
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/nat/nat_type.h"
#include "hypersomnia_version.h"
#include "application/masterserver/server_request_signature.h"
#include "application/masterserver/server_instance_label.h"

bool operator==(const netcode_address_t& a, const netcode_address_t& b);
bool operator!=(const netcode_address_t& a, const netcode_address_t& b);

enum class ranked_server_type : uint8_t {
	NONE,
	JOINABLE,
	LIVE,
	TOURNAMENT_BYES,
	TOURNAMENT_MID,
	TOURNAMENT_SEMIFINALS,
	TOURNAMENT_FINALS
};

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
	uint8_t server_slots;

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

	uint8_t ranked_state = 0;
	bool require_password = false;

	uint8_t speed_hundredths = 100;
	uint8_t bullet_speed_hundredths = 100;
	uint8_t zoom_hundredths = 100;

	host_string_type domain;
	instance_label_type instance_label;
	heartbeat_nonce_type ack_nonce = {};
	std::optional<server_request_signature> signature;
	// END GEN INTROSPECTOR

	/*
		The game speed, the bullet speed and the zoom the players currently get,
		all quantized to 0.01 in the game, so in hundredths they fit a byte (up to 2.55).
	*/

	static uint8_t to_hundredths(const real32 value) {
		if (!std::isfinite(value)) {
			return 0;
		}

		return static_cast<uint8_t>(std::clamp(std::round(value * 100.f), 0.f, 255.f));
	}

	void set_mults(const real32 speed, const real32 bullet_speed, const real32 zoom) {
		speed_hundredths = to_hundredths(speed);
		bullet_speed_hundredths = to_hundredths(bullet_speed);
		zoom_hundredths = to_hundredths(zoom);
	}

	double cached_time_to_event = 0.0;

	bool any_human() const {
		return num_online_humans > 0;
	}

	uint8_t num_online_bots() const {
		return num_online - num_online_humans;
	}

	uint8_t max_online() const {
		return server_slots + num_online_bots();
	}

	ranked_server_type get_ranked_type() const {
		return static_cast<ranked_server_type>(ranked_state);
	}

	void set_ranked_type(const ranked_server_type t) {
		ranked_state = static_cast<uint8_t>(t);
	}

	bool is_ranked_server() const {
		return get_ranked_type() != ranked_server_type::NONE;
	}

	bool is_still_joinable_ranked() const {
		return get_ranked_type() == ranked_server_type::JOINABLE;
	}

	bool is_tournament_server() const {
		const auto t = get_ranked_type();
		return t == ranked_server_type::TOURNAMENT_BYES
			|| t == ranked_server_type::TOURNAMENT_MID
			|| t == ranked_server_type::TOURNAMENT_SEMIFINALS
			|| t == ranked_server_type::TOURNAMENT_FINALS;
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

	bool versions_compatible() const;
};
