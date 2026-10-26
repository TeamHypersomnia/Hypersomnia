#pragma once

/*
	A tournament round's timeout: by bracket_round_deadline (a fixed moment - round 1: the tournament's start + its timeout,
	the next rounds: the previous round's end + theirs), a match still waiting for its players is decided without them:

	- an open match of round 1 closes (close_open_match_if_requested):
	  whoever waits goes on;
	- a match of known teams: a team with nobody on its server is out (both - nobody came); both teams with someone
	  there play with whoever is there.

	Applied on every tick past it until the match starts - a leave in the seconds before the start restarts the match,
	and the rule applies again. A match being played, or about to start, is never touched.

	Close the round (the administration panel, any of the round's servers): the same, now, on all of them.
*/

bool server_setup::join_timeout_due() const {
	if (close_bracket_round != nullptr && close_bracket_round->load()) {
		return true;
	}

	return
		bracket_round_deadline > 0
		&& has_assigned_teams()
		&& static_cast<int64_t>(augs::date_time::secs_since_epoch()) >= bracket_round_deadline
	;
}

void server_setup::check_join_timeout() {
	if (shutdown_scheduled || !join_timeout_due() || is_open_match() || is_ranked_live_or_starting()) {
		return;
	}

	decide_by_present_players();
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

	/*
		Both teams with someone here: they play with whoever is here - the match starts once both sides are in the game
		(all_assigned_present). Said once.
	*/

	if (resistance > 0 && metropolis > 0) {
		if (!plays_with_whoever_came) {
			LOG("Round timeout: %x and %x players came - the match starts with them.", resistance, metropolis);
			plays_with_whoever_came = true;
		}

		return;
	}

	const auto winner =
		resistance > 0 ? faction_type::RESISTANCE :
		metropolis > 0 ? faction_type::METROPOLIS :
		faction_type::COUNT
	;

	LOG("Round timeout: %x.", winner == faction_type::COUNT ? "nobody came - both teams are out" : "a team with nobody here is out - the other wins by walkover");

	if (on_join_timeout) {
		on_join_timeout(winner);
	}

	schedule_shutdown();
}
