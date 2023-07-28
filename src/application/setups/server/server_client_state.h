#pragma once
#include "application/setups/server/server_vars.h"
#include "augs/network/jitter_buffer.h"
#include "augs/network/network_types.h"
#include "game/modes/mode_entropy.h"

#include "application/network/requested_client_settings.h"
#include "application/network/client_state_type.h"

#include "view/mode_gui/arena/arena_player_meta.h"

using client_pending_entropies = std::vector<total_client_entropy>;

enum class downloading_type {
	NONE,
	EXTERNALLY,
	DIRECTLY
};

struct server_client_state {
	using type = client_state_type;

	type state = type::NETCODE_NEGOTIATING_CONNECTION;
	net_time_t last_valid_payload_time = -1.0;
	net_time_t last_keyboard_activity_time = -1.0;

	net_time_t when_last_sent_file_packet = 0.0f;

	std::optional<augs::secure_hash_type> now_downloading_file;

	requested_client_settings settings;
	bool rebroadcast_public_settings = false;

	client_pending_entropies pending_entropies;
	uint8_t num_entropies_accepted = 0;

	unsigned resyncs_counter = 0;
	net_time_t last_resync_counter_reset_at = 0;
	unsigned unauthorized_rcon_commands = 0;
	std::optional<net_time_t> when_kicked;

	std::string uploaded_avatar_url;
	bool pushed_connected_webhook = false;

	downloading_type downloading_status = downloading_type::NONE;

	arena_player_meta meta;

	uint32_t direct_file_chunks_left = 0;

	server_client_state() = default;
	server_client_state(const net_time_t server_time) {
		init(server_time);
	}

	void reset_solvable_stream() {
		num_entropies_accepted = 0;
		pending_entropies.clear();
	}

	bool should_move_to_spectators_due_to_afk(const server_vars& v, const net_time_t server_time) const {
		if (downloading_status != downloading_type::NONE) {
			return true;
		}

		const auto diff = server_time - last_keyboard_activity_time;
		const auto bare_minimum_afk = 10u;

		return diff > std::max(bare_minimum_afk, v.move_to_spectators_if_afk_for_secs);
	}

	bool should_kick_due_to_afk(const server_vars& v, const net_time_t server_time) const {
		if (downloading_status != downloading_type::NONE) {
			return false;
		}

		const auto diff = server_time - last_keyboard_activity_time;
		const auto bare_minimum_afk = 10u;

		return diff > std::max(bare_minimum_afk, v.kick_if_afk_for_secs);
	}

	bool should_kick_due_to_inactivity(const server_vars& v, const net_time_t server_time) const {
		if (downloading_status != downloading_type::NONE) {
			return false;
		}

		const auto diff = server_time - last_valid_payload_time;

		if (state == type::IN_GAME) {
			const auto bare_minimum_limit = 2u;
			return diff > std::max(bare_minimum_limit, v.kick_if_no_network_payloads_for_secs);
		}

		const auto bare_minimum_limit = 5u;
		return diff > std::max(bare_minimum_limit, v.time_limit_to_enter_game_since_connection);
	}

	void set_in_game(const net_time_t at_time) {
		state = type::IN_GAME;
		last_valid_payload_time = at_time;
		last_keyboard_activity_time = at_time;
	}

	bool is_set() const {
		return state != type::NETCODE_NEGOTIATING_CONNECTION;
	}

	void init(const net_time_t server_time) {
		ensure(!is_set());

		state = type::PENDING_WELCOME;
		last_valid_payload_time = server_time;
	}

	void unset() {
		*this = {};
	}

	bool should_pause_solvable_stream() const {
		return downloading_status != downloading_type::NONE; 
	}

	std::string get_nickname() const {
		return settings.chosen_nickname;
	}
};
