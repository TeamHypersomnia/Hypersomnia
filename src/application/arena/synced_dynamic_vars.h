#pragma once
#include <cstdint>
#include "application/setups/server/server_vars.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/bot_quota.h"

struct bots_request {
	// GEN INTROSPECTOR struct bots_request
	mode_player_id requester;
	bot_quota quota;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return !quota.empty();
	}

	bool operator==(const bots_request&) const = default;
};

struct bot_difficulty_request {
	mode_player_id requester;
	difficulty_type difficulty = difficulty_type::PROGRESSIVE;

	bool is_set() const {
		return requester.is_set();
	}
};

/*
	A multiplier requested by a player with a chat command - /speed, /bspeed or /zoom.
*/

struct mult_request {
	mode_player_id requester;
	real32 value = 1.f;

	bool is_set() const {
		return requester.is_set();
	}
};

struct synced_dynamic_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct synced_dynamic_vars
	bool preassigned_factions = false;
	bool all_assigned_present = false;
	bool all_authenticated = false;
	bool all_not_banned = false;
	bool friendly_fire = false;
	bool freeze_time = true;
	augs::maybe<real32> zoom_override;
	bool apply_zoom_immediately = false;
	bool force_short_match = false;
	bool allow_overtime = false;
	bool shutdown_after_one_match = false;
	augs::maybe<uint32_t> max_team_score;
	server_ranked_vars ranked;
	bots_request bots_override;
	difficulty_type bot_override_difficulty = difficulty_type::PROGRESSIVE;
	real32 logic_speed = 1.f;
	real32 bullet_speed = 1.f;
	bool apply_logic_speed_immediately = false;
	// END GEN INTROSPECTOR

	bool operator==(const synced_dynamic_vars&) const = default;

	bool is_ranked_server() const {
		return ranked.is_ranked_server();
	}

	/*
		For local setups (test scene, editor playtest).
		The bullet speed follows the game speed unless set separately.
	*/

	void set_speeds(const real32 game_speed, const augs::maybe<real32>& separate_bullet_speed) {
		logic_speed = game_speed;
		bullet_speed = separate_bullet_speed.is_enabled ? separate_bullet_speed.value : game_speed;
	}
};

