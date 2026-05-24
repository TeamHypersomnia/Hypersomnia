#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path_declaration.h"

/*
	Player identifier in tournament state. Same shape server_assigned_teams uses
	as the key of id_to_faction. Aliased so the intent reads at the call site
	instead of "raw string".
*/
using account_id_string = std::string;

struct tournament_team_state {
	// GEN INTROSPECTOR struct tournament_team_state
	std::vector<account_id_string> player_ids;
	float skill = 0.f;
	bool eliminated = false;
	// END GEN INTROSPECTOR
};

struct tournament_match {
	// GEN INTROSPECTOR struct tournament_match
	uint32_t team_a_index = 0;
	uint32_t team_b_index = 0;
	std::vector<account_id_string> team_a_player_ids;
	std::vector<account_id_string> team_b_player_ids;
	std::vector<account_id_string> spectator_player_ids;
	uint16_t port = 0;
	std::string server_name;
	std::string arena_and_mode;

	bool resolved = false;
	uint32_t winner_team_index = 0;
	float duration_secs = 0.f;
	uint8_t heartbeat_ranked_type = 0;
	// END GEN INTROSPECTOR
};

struct tournament_match_history_entry {
	// GEN INTROSPECTOR struct tournament_match_history_entry
	uint32_t played_in_stage = 0;
	std::vector<account_id_string> winner_player_ids;
	std::vector<account_id_string> loser_player_ids;
	float duration_secs = 0.f;
	// END GEN INTROSPECTOR
};

struct tournament_state {
	// GEN INTROSPECTOR struct tournament_state
	uint32_t stage_index = 0;
	std::vector<tournament_team_state> teams;
	std::vector<tournament_match> current_stage_matches;
	std::vector<tournament_match_history_entry> match_history;
	bool initialized = false;
	std::string config_hash;
	// END GEN INTROSPECTOR

	void save(const augs::path_type& path) const;
	static std::optional<tournament_state> load(const augs::path_type& path);

	std::vector<uint32_t> surviving_team_indices() const;
	bool tournament_finished() const;
	std::optional<uint32_t> sole_surviving_team_index() const;
};
