#pragma once

/*
	Who watches a match being played (a ranked match, a tournament's match) - its spectators:

	- whoever was a spectator when it started (match_start_spectators) - back and forth as they like;
	- an admin (admin_profiles), the spectator account (a caster, a stream bot);
	- in a tournament: who's on this match's roster as a spectator, or a team with no match of its own to play now
	  (may_spectate_tournament).

	They come in also while the server refuses new clients - by who they are, in the handshake already (netcode's
	accept_while_refusing) - never take a team, and the players never hear of them: no "connected", no chat
	(see arena_mode's notifications and server_setup::broadcast).
*/

bool server_setup::is_spectator_account(const std::string& account) const {
	return !private_vars.spectator_account.empty() && ::normalize_account_id(private_vars.spectator_account) == ::normalize_account_id(account);
}

/*
	admin_account is what the cert proves of an administrator (admin_account_of) - besides the account it plays as.
*/

bool server_setup::may_watch_running_match(const std::string& account, const std::string& admin_account) const {
	if (!admin_account.empty()) {
		if (find_admin_profile_of(admin_account).has_value() || is_spectator_account(admin_account)) {
			return true;
		}
	}

	if (account.empty()) {
		return false;
	}

	if (find_admin_profile_of(account).has_value() || is_spectator_account(account)) {
		return true;
	}

	if (found_in(match_start_spectators, account)) {
		return true;
	}

	if (const auto assigned = mapped_or_nullptr(assigned_teams.id_to_faction, account); assigned != nullptr && *assigned == faction_type::SPECTATOR) {
		return true;
	}

	return may_spectate_tournament && may_spectate_tournament(account);
}

/*
	A player of the match being played - in a team, connected or suspended. Comes back also before the server noticed
	the old connection gone: the newer connection then replaces it (apply_authentication).
*/

bool server_setup::plays_running_match(const std::string& account) const {
	if (account.empty()) {
		return false;
	}

	if (is_currently_suspended(account)) {
		return true;
	}

	return get_arena_handle().on_mode(
		[&](const auto& mode) {
			using M = std::remove_cvref_t<decltype(mode)>;

			auto found = false;

			if constexpr(std::is_same_v<M, arena_mode>) {
				for (const auto faction : { faction_type::RESISTANCE, faction_type::METROPOLIS }) {
					mode.for_each_player_in(faction, [&](const auto, const auto& player) {
						if (player.server_ranked_account_id == account) {
							found = true;
						}
					});
				}
			}

			return found;
		}
	);
}

/*
	The handshake of a client while the match refuses new ones: its identity checked as on connecting
	(verify_client_identity), then whether it may come in - a player of the match back, or one who watches.
*/

bool server_setup::accept_while_match_refuses(const uint8_t* const binding, const uint8_t* const session_signature, const uint8_t* const user_data) const {
	const auto blob = identity_cert::read_blob(user_data);

	if (!blob.has_value() || !blob->identity.has_value()) {
		return false;
	}

	const auto verified = identity_cert::verify(
		*blob->identity,
		trusted_broker_keys,
		static_cast<int64_t>(augs::date_time::secs_since_epoch()),
		binding,
		session_signature,
		nullptr
	);

	if (!verified.has_value() || is_banned(verified->account)) {
		return false;
	}

	const auto& account = verified->account;
	return plays_running_match(account) || may_watch_running_match(account, ::admin_account_of(*blob->identity));
}

/*
	Who may play on this server: a player of its roster or of the match being played, anyone in an open match with places
	left - and, while a roster by nicknames isn't all claimed, anyone (who it is is known only once the nickname comes).
*/

bool server_setup::may_play_here(const std::string& account) const {
	if (::is_actual_faction(get_assigned_team(account)) || plays_running_match(account)) {
		return true;
	}

	if (may_spectate_tournament && may_spectate_tournament(account)) {
		return false;
	}

	if (is_open_match() && !is_ranked_live_or_starting()) {
		std::size_t taken = 0;

		for (const auto& [id, faction] : assigned_teams.id_to_faction) {
			(void)id;
			taken += ::is_actual_faction(faction) ? 1 : 0;
		}

		if (taken < 2 * assigned_teams.open_team_size) {
			return true;
		}
	}

	for (const auto& [id, faction] : assigned_teams.id_to_faction) {
		if (begins_with(id, nickname_account_id_prefix_v)) {
			return true;
		}
	}

	return false;
}

/*
	A tournament's match: at most tournament_spectator_places others watch it at once - every connection takes RAM
	and upload. Never its players, nor the admins and the spectator account (places of their own).
*/

bool server_setup::no_place_to_watch(const client_id_type id) const {
	if (!tournament_spectator_places.has_value() || !has_assigned_teams()) {
		return false;
	}

	auto watches = [&](const server_client_state& c) {
		const bool own_place =
			find_admin_profile_of(c.admin_account).has_value()
			|| is_spectator_account(c.admin_account)
			|| is_spectator_account(c.authenticated_id)
		;

		return !own_place && !may_play_here(c.authenticated_id);
	};

	if (!watches(clients[id])) {
		return false;
	}

	uint32_t watching = 0;

	for_each_id_and_client([&](const auto other_id, const auto& other) {
		if (other_id != id && other.is_set() && !other.when_kicked.has_value() && watches(other)) {
			++watching;
		}
	}, only_connected_v);

	return watching >= *tournament_spectator_places;
}

/*
	The spectators when a match starts may come and go while it's played; a new match starts the list anew.
*/

void server_setup::track_match_start_spectators() {
	const bool running = is_ranked_live_or_starting();

	if (running && !match_was_running) {
		match_start_spectators.clear();

		for_each_id_and_client([&](const auto id, const auto& c) {
			if (c.is_authenticated() && get_assigned_faction(to_mode_player_id(id)) == faction_type::SPECTATOR) {
				match_start_spectators.push_back(c.authenticated_id);
			}
		}, connected_and_integrated_v);
	}

	if (!running) {
		match_start_spectators.clear();
	}

	match_was_running = running;
}

/*
	The spectator account never takes a team - its team choices are dropped.
*/

total_client_entropy server_setup::without_spectator_account_team_choice(const mode_player_id mode_id, const total_client_entropy& entropy) const {
	if (!std::holds_alternative<mode_commands::team_choice>(entropy.mode)) {
		return entropy;
	}

	const auto c = find_client_state(mode_id);

	if (c == nullptr || !(is_spectator_account(c->authenticated_id) || is_spectator_account(c->admin_account))) {
		return entropy;
	}

	auto filtered = entropy;
	filtered.mode = std::monostate();
	return filtered;
}
