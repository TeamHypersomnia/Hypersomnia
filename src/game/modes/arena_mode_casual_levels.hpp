#pragma once

bool arena_mode::casual_levels_enabled(const_input_type in) const {
	if (in.rules.is_gun_game()) {
		return false;
	}

	if (in.is_ranked_server()) {
		return false;
	}

	if (in.dynamic_vars.bot_override_difficulty != difficulty_type::LEVELLING) {
		return false;
	}

	return true;
}

uint16_t arena_mode::calc_team_level(const faction_type f) const {
	if (f == faction_type::SPECTATOR) {
		return 0;
	}

	uint32_t sum = 0;
	uint32_t count = 0;

	for (const auto& p : only_human(players)) {
		if (p.second.get_faction() == f) {
			sum += p.second.session.casual_level;
			++count;
		}
	}

	if (count == 0) {
		return 0;
	}

	return uint16_t(sum / count);
}

per_actual_faction<uint8_t> arena_mode::calc_requested_bots_from_casual_levels(
	const const_input_type in,
	const uint8_t team_size
) const {
	per_actual_faction<uint8_t> result = { 0u, 0u, 0u };

	const auto p = calc_participating_factions(in);

	if (!p.valid() || team_size == 0) {
		return result;
	}

	const auto factions_arr = p.get_all();

	/*
		Detect PvE: only one team has humans. Use the level "pressure" composition
		directly: ally bots thin out as the human team progresses, excess pressure
		spills onto the bot-only side.

		PvP: simulate a PvE scenario where the stronger team is the only one
		playing, at virtual level DT = |L_stronger - L_weaker|. The weaker team's
		fictional PvE bot count is then reduced by the weaker team's actual
		human count. Small DT keeps bots mild even at high baseline level; huge
		DT escalates support for the weaker side.
	*/
	std::size_t factions_with_humans = 0;

	for (const auto t : factions_arr) {
		if (num_human_players_in(t) > 0) {
			++factions_with_humans;
		}
	}

	const bool is_pvp = factions_with_humans >= 2;

	if (is_pvp) {
		const auto fa = factions_arr[0];
		const auto fb = factions_arr[1];
		const auto lvl_a = calc_team_level(fa);
		const auto lvl_b = calc_team_level(fb);

		const auto stronger = lvl_a >= lvl_b ? fa : fb;
		const auto weaker = stronger == fa ? fb : fa;
		const auto dt = uint16_t(std::max(lvl_a, lvl_b) - std::min(lvl_a, lvl_b));

		const auto humans_stronger = uint8_t(std::min<std::size_t>(team_size, num_human_players_in(stronger)));
		const auto humans_weaker = uint8_t(std::min<std::size_t>(team_size, num_human_players_in(weaker)));

		{
			casual_level_alloc_input ai;
			ai.team_size = team_size;
			ai.num_humans_own = humans_stronger;
			ai.num_humans_opp = 0;
			ai.level_own = dt;
			ai.level_opp = 0;
			result[stronger] = casual_level_alloc_for_team(ai);
		}

		{
			casual_level_alloc_input ai;
			ai.team_size = team_size;
			ai.num_humans_own = 0;
			ai.num_humans_opp = humans_stronger;
			ai.level_own = 0;
			ai.level_opp = dt;
			const auto fictional_weaker_bots = int(casual_level_alloc_for_team(ai));

			result[weaker] = uint8_t(std::max(0, fictional_weaker_bots - int(humans_weaker)));
		}
	}
	else {
		for (const auto t : factions_arr) {
			const auto opp = p.get_opposing(t);

			casual_level_alloc_input ai;
			ai.team_size = team_size;
			ai.num_humans_own = uint8_t(std::min<std::size_t>(team_size, num_human_players_in(t)));
			ai.num_humans_opp = uint8_t(std::min<std::size_t>(team_size, num_human_players_in(opp)));
			ai.level_own = calc_team_level(t);
			ai.level_opp = calc_team_level(opp);

			result[t] = casual_level_alloc_for_team(ai);
		}
	}

	return verify_max_quota(result);
}

difficulty_type arena_mode::calc_bot_difficulty(const_input_type in) const {
	const auto override_diff = in.dynamic_vars.bot_override_difficulty;

	if (override_diff != difficulty_type::LEVELLING) {
		return override_diff;
	}

	if (!casual_levels_enabled(in)) {
		return difficulty_type::VERY_EASY;
	}

	const auto p = calc_participating_factions(in);

	if (!p.valid()) {
		return difficulty_type::VERY_EASY;
	}

	const auto humans_bombing = num_human_players_in(p.bombing);
	const auto humans_defusing = num_human_players_in(p.defusing);

	if (humans_bombing > 0 && humans_defusing > 0) {
		/*
			PvP: difficulty scales with the skill delta (DT) between the two
			human teams, not with their absolute levels. Closely-matched teams
			get easy bots regardless of baseline; massive skill gaps escalate.
		*/
		const auto lvl_b = calc_team_level(p.bombing);
		const auto lvl_d = calc_team_level(p.defusing);
		const auto dt = uint16_t(std::max(lvl_b, lvl_d) - std::min(lvl_b, lvl_d));
		return bot_difficulty_for_casual_level(dt);
	}

	/*
		PvE: all bots match the (single) human team's level.
	*/
	const auto human_team =
		humans_bombing > 0 ?
		p.bombing :
		p.defusing
	;

	return bot_difficulty_for_casual_level(calc_team_level(human_team));
}

