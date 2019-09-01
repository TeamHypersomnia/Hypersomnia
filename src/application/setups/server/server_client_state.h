#pragma once
#include "application/setups/server/server_vars.h"
#include "augs/network/jitter_buffer.h"
#include "augs/network/network_types.h"
#include "game/modes/mode_entropy.h"

#include "application/network/requested_client_settings.h"
#include "application/network/client_state_type.h"

#include "view/mode_gui/arena/arena_player_meta.h"

using client_pending_entropies = std::vector<total_client_entropy>;

struct server_client_state {
	using type = client_state_type;

	type state = type::INITIATING_CONNECTION;
	net_time_t last_valid_message_time = -1.0;
	net_time_t last_keyboard_activity_time = -1.0;
	requested_client_settings settings;
	bool rebroadcast_public_settings = false;

	client_pending_entropies pending_entropies;
	uint8_t num_entropies_accepted = 0;

	unsigned resyncs_counter = 0;
	net_time_t last_resync_counter_reset_at = 0;
	unsigned unauthorized_rcon_commands = 0;
	std::optional<net_time_t> when_kicked;

	arena_player_meta meta;

	server_client_state() = default;

	server_client_state(const net_time_t server_time) {
		init(server_time);
	}

	bool should_kick_due_to_afk(const server_vars& v, const net_time_t server_time) const {
		const auto diff = server_time - last_keyboard_activity_time;
		return diff > std::max(20u, v.kick_if_away_from_keyboard_for_secs);
	}

	bool should_kick_due_to_inactivity(const server_vars& v, const net_time_t server_time) const {
		const auto diff = server_time - last_valid_message_time;

		if (state == type::IN_GAME) {
			return diff > std::max(2u, v.kick_if_no_messages_for_secs);
		}

		return diff > std::max(5u, v.time_limit_to_enter_game_since_connection);
	}

	void set_in_game(const net_time_t at_time) {
		state = type::IN_GAME;
		last_valid_message_time = at_time;
		last_keyboard_activity_time = at_time;
	}

	bool is_set() const {
		return state != type::INITIATING_CONNECTION;
	}

	void init(const net_time_t server_time) {
		ensure(!is_set());

		state = type::PENDING_WELCOME;
		last_valid_message_time = server_time;
	}

	void unset() {
		*this = {};
	}
};
