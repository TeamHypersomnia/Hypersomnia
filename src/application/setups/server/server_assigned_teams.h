#pragma once

struct server_assigned_teams_json {
	// GEN INTROSPECTOR struct server_assigned_teams_json
	std::vector<std::string> metropolis;
	std::vector<std::string> resistance;
	std::vector<std::string> atlantis;
	std::vector<std::string> spectators;
	// END GEN INTROSPECTOR
};

struct server_assigned_teams {
	std::unordered_map<
		std::string,
		faction_type
	> id_to_faction;

	/*
		An open match of a tournament's first round: nobody is assigned beforehand - the first 2 * open_team_size
		players to join make the match, assigned to the two teams by turns.
	*/

	uint32_t open_team_size = 0;

	server_assigned_teams() = default;
	server_assigned_teams(const augs::path_type&);
};

