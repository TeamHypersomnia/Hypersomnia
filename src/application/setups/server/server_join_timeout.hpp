#pragma once

/*
	A tournament match's join timeout - join_timeout_secs after the round's first player came (to any of its
	matches), a match still waiting for its players is decided without them:

	- an open match of round 1 closes as if Close round 1 were clicked (close_open_match_if_requested):
	  whoever waits goes on;
	- a match of known teams: the team with more players present wins by walkover, nobody - both out,
	  equal numbers - it starts with them (join_timeout_relaxed lifts the "every listed player" requirement).

	A match being played, or about to start, is never touched.
*/

/*
	Counted from the moment the first player with a team in this round came to any of its matches (round_first_arrival,
	shared by the round's servers) - not from the servers' start: a tournament's server may be started long before
	the players come (installed as a service), and its matches must not run out with nobody there yet.
	server_time is the process's clock, the same for all the round's servers. Without a round to share it with,
	from this server's first tick.
*/

bool server_setup::join_timeout_due() const {
	if (timeout_requested) {
		return true;
	}

	return
		join_timeout_secs > 0
		&& has_assigned_teams()
		&& join_timeout_since.has_value()
		&& server_time - *join_timeout_since >= static_cast<double>(join_timeout_secs)
	;
}

void server_setup::check_join_timeout() {
	if (!join_timeout_since.has_value()) {
		if (round_first_arrival == nullptr) {
			join_timeout_since = server_time;
		}
		else {
			auto a_player_is_here = [&]() {
				bool here = false;

				for_each_id_and_client([&](auto, const auto& c) {
					if (c.is_authenticated() && ::is_actual_faction(get_assigned_team(c.authenticated_id))) {
						here = true;
					}
				}, only_connected_v);

				return here;
			};

			if (round_first_arrival->load() < 0.0 && a_player_is_here()) {
				auto none = -1.0;
				round_first_arrival->compare_exchange_strong(none, server_time);
			}

			if (const auto came_at = round_first_arrival->load(); came_at >= 0.0) {
				join_timeout_since = came_at;
			}
		}
	}

	if (join_timeout_passed || !join_timeout_due() || is_open_match()) {
		return;
	}

	join_timeout_passed = true;

	if (is_ranked_live_or_starting()) {
		return;
	}

	decide_by_present_players();
}

/*
	"Timeout absent players" (the administration panel): what the join timeout would do, now - an open match
	closes as by Close round 1 (close_open_match_if_requested reads timeout_requested), one of known teams is decided.
*/

bool server_setup::timeout_absent_players_now() {
	if (!has_assigned_teams() || is_ranked_live_or_starting() || join_timeout_passed || open_match_closed) {
		return false;
	}

	timeout_requested = true;

	if (!is_open_match()) {
		join_timeout_passed = true;
		decide_by_present_players();
	}

	return true;
}

void server_setup::decide_by_present_players() {
	auto resistance = std::size_t(0);
	auto metropolis = std::size_t(0);

	/*
		Present: connected and known - also one still downloading the arena.
	*/

	for_each_id_and_client([&](auto, const auto& c) {
		if (!c.is_authenticated()) {
			return;
		}

		const auto faction = get_assigned_team(c.authenticated_id);

		resistance += faction == faction_type::RESISTANCE ? 1 : 0;
		metropolis += faction == faction_type::METROPOLIS ? 1 : 0;
	}, connected_and_integrated_v);

	LOG("Join timeout: %x and %x players came.", resistance, metropolis);

	if (resistance == metropolis && resistance > 0) {
		LOG("Join timeout: equal numbers - the match starts with them.");
		join_timeout_relaxed = true;
		return;
	}

	const auto winner =
		resistance == metropolis ? faction_type::COUNT :
		resistance > metropolis ? faction_type::RESISTANCE :
		faction_type::METROPOLIS
	;

	LOG("Join timeout: %x.", winner == faction_type::COUNT ? "nobody came - both teams are out" : "the team that came wins by walkover");

	if (on_join_timeout) {
		on_join_timeout(winner);
	}

	schedule_shutdown();
}
