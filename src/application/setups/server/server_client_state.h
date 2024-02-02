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

using server_client_session_id = uint32_t; 

struct server_client_state {
	using type = client_state_type;

	type state = type::NETCODE_NEGOTIATING_CONNECTION;
	net_time_t when_connected = -1.0;

	net_time_t last_valid_payload_time = -1.0;
	net_time_t last_keyboard_activity_time = -1.0;

	net_time_t when_last_sent_file_packet = 0.0f;
	net_time_t when_sent_auth_ticket = -1.0;

	std::optional<augs::secure_hash_type> now_downloading_file;

	requested_client_settings settings;
	bool rebroadcast_synced_meta = false;

	client_pending_entropies pending_entropies;
	uint8_t num_entropies_accepted = 0;

	unsigned resyncs_counter = 0;
	net_time_t last_resync_counter_reset_at = 0;
	unsigned unauthorized_rcon_commands = 0;
	std::optional<net_time_t> when_kicked;
	bool kick_no_linger = false;

	std::string uploaded_avatar_url;
	bool pushed_connected_webhook = false;
	bool lingering_after_arena_reloaded = false;

	downloading_type downloading_status = downloading_type::NONE;

	arena_player_meta meta;

	uint32_t direct_file_chunks_left = 0;

	bool auth_requested = false;
	std::string authenticated_id;
	bool verified_has_no_ban = false;

	server_client_session_id session_id = 0;

	server_client_state() = default;

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

	bool should_kick_due_to_network_timeout(const server_vars& v, const net_time_t server_time) const {
		if (downloading_status != downloading_type::NONE) {
			return false;
		}

		const auto diff = server_time - last_valid_payload_time;

		if (state == type::IN_GAME) {
			const auto bare_minimum_limit = 0.2f;
			return diff > std::max(bare_minimum_limit, v.get_client_network_timeout_secs());
		}

		const auto bare_minimum_limit = 2u;
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

	void init(const net_time_t server_time, const uint32_t new_session_id) {
		ensure(!is_set());

		state = type::PENDING_WELCOME;
		last_valid_payload_time = server_time;
		when_connected = server_time;
		session_id = new_session_id;
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

	bool is_authenticated() const {
		return !authenticated_id.empty();
	}

	bool should_kick_due_to_unauthenticated(const server_vars& v, const net_time_t server_time) const {
		if (downloading_status != downloading_type::NONE) {
			return false;
		}

		if (!v.requires_authentication()) {
			return false;
		}

		if (is_authenticated()) {
			return false;
		}

		if (when_sent_auth_ticket != -1.0) {
			const auto diff = server_time - when_sent_auth_ticket;
			return diff > std::max(0.0f, v.kick_if_unauthenticated_for_secs);
		}

		const auto diff = server_time - when_connected;
		return diff > std::max(0.0f, v.kick_if_unauthenticated_for_secs);
	}

	net_time_t secs_since_connected(const net_time_t server_time) const {
		if (when_connected < 0.0) {
			return 0.0;
		}

		return server_time - when_connected;
	}

	auto make_synced_meta() const {
		synced_player_meta new_meta;

		new_meta.public_settings = settings.public_settings;
		new_meta.authenticated = is_authenticated();

		return new_meta;
	}
};
