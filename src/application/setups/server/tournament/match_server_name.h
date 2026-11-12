#pragma once
#include <string>
#include <vector>
#include "augs/network/network_types.h"
#include "augs/string/utf8_prefix.h"

/*
	A match's server name by its teams' nicknames - "[alice, bob] vs [carol, dave]", as the players look for theirs.
	Longer than a server name may be: each team by its first player and how many more - "[alice +3] vs [eve +3]";
	still too long, the first nicknames cut, never inside a character.
*/

inline std::string match_server_name(const std::vector<std::string>& team_a, const std::vector<std::string>& team_b) {
	auto whole_team = [](const std::vector<std::string>& team) {
		std::string s = "[";

		for (std::size_t i = 0; i < team.size(); ++i) {
			s += (i > 0 ? ", " : "") + team[i];
		}

		return s + "]";
	};

	const auto whole = whole_team(team_a) + " vs " + whole_team(team_b);

	if (whole.size() <= max_server_name_length_v) {
		return whole;
	}

	auto more = [](const std::vector<std::string>& team) {
		return team.size() > 1 ? " +" + std::to_string(team.size() - 1) : std::string();
	};

	auto first = [](const std::vector<std::string>& team) {
		return team.empty() ? std::string() : team.front();
	};

	const auto more_a = more(team_a);
	const auto more_b = more(team_b);
	const auto frame = std::string("[") + more_a + "] vs [" + more_b + "]";
	const auto room = max_server_name_length_v > frame.size() ? max_server_name_length_v - frame.size() : std::size_t(0);

	auto first_a = first(team_a);
	auto first_b = first(team_b);

	if (first_a.size() + first_b.size() > room) {
		const auto half = room / 2;

		if (first_a.size() <= half) {
			first_b = augs::utf8_prefix_never_cut_inside_a_character(first_b, room - first_a.size());
		}
		else if (first_b.size() <= half) {
			first_a = augs::utf8_prefix_never_cut_inside_a_character(first_a, room - first_b.size());
		}
		else {
			first_a = augs::utf8_prefix_never_cut_inside_a_character(first_a, half);
			first_b = augs::utf8_prefix_never_cut_inside_a_character(first_b, room - half);
		}
	}

	return "[" + first_a + more_a + "] vs [" + first_b + more_b + "]";
}
