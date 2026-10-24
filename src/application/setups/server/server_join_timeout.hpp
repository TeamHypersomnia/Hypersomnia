#pragma once

/*
	A tournament match's join timeout - join_timeout_secs after the server started, a match still waiting
	for its players is decided without them:

	- an open match of round 1 closes as if Close round 1 were clicked (close_open_match_if_requested):
	  whoever waits goes on;
	- a match of known teams: the team with more players present wins by walkover, nobody - both out,
	  equal numbers - it starts with them (join_timeout_relaxed lifts the "every listed player" requirement).

	A match being played, or about to start, is never touched.
*/

/*
	Counted from this match server's first tick - server_time is the process's clock, not this server's.
*/

bool server_setup::join_timeout_due() const {
	return
		join_timeout_secs > 0
		&& has_assigned_teams()
		&& join_timeout_since.has_value()
		&& server_time - *join_timeout_since >= static_cast<double>(join_timeout_secs)
	;
}

void server_setup::check_join_timeout() {
	if (!join_timeout_since.has_value()) {
		join_timeout_since = server_time;
	}

	if (join_timeout_passed || !join_timeout_due() || is_open_match()) {
		return;
	}

	join_timeout_passed = true;

	if (is_ranked_live_or_starting()) {
		return;
	}

	std::size_t resistance = 0;
	std::size_t metropolis = 0;

	for_each_id_and_client([&](auto, const auto& c) {
		if (c.state != client_state_type::IN_GAME) {
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
