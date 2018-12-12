#pragma once
#include "application/setups/server/server_vars.h"
#include "augs/network/network_types.h"
#include "game/modes/mode_entropy.h"

#include "application/network/requested_client_settings.h"

using client_pending_entropies = std::vector<total_client_entropy>;

struct server_client_state {
	enum class type {
		INVALID,

		PENDING_WELCOME,
		RECEIVING_INITIAL_STATE,
		RECEIVING_INITIAL_STATE_CORRECTION,
		IN_GAME
	};

	type state = type::INVALID;
	net_time_t last_valid_activity_time = -1.0;
	requested_client_settings settings;

	client_pending_entropies entropies;

	server_client_state() = default;

	server_client_state(const net_time_t server_time) {
		init(server_time);
	}

	bool should_kick_due_to_inactivity(const server_vars& v, const net_time_t server_time) const {
		const auto diff = server_time - last_valid_activity_time;

		if (state == type::IN_GAME) {
			return diff > v.kick_if_inactive_for_secs;
		}

		return diff > v.time_limit_to_enter_game_since_connection;
	}

	void set_in_game(const net_time_t at_time) {
		state = type::IN_GAME;
		last_valid_activity_time = at_time;
	}

	bool is_set() const {
		return state != type::INVALID;
	}

	void init(const net_time_t server_time) {
		state = type::PENDING_WELCOME;
		last_valid_activity_time = server_time;
		settings = {};
		entropies.clear();
	}

	void unset() {
		state = type::INVALID;
		entropies.clear();
	}
};
