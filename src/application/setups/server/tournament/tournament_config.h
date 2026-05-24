#pragma once
#include <string>
#include <vector>
#include "augs/filesystem/path_declaration.h"

enum class tournament_skill_source {
	// GEN INTROSPECTOR enum class tournament_skill_source
	ORDER,
	RANDOM,
	ENDPOINT,

	COUNT
	// END GEN INTROSPECTOR
};

enum class tournament_matchup_scheme {
	// GEN INTROSPECTOR enum class tournament_matchup_scheme
	STRONGEST_VS_WEAKEST,
	STRONGEST_VS_STRONGEST,

	COUNT
	// END GEN INTROSPECTOR
};

struct tournament_config {
	// GEN INTROSPECTOR struct tournament_config
	tournament_skill_source skill_level_source = tournament_skill_source::ORDER;
	bool skill_by_match_time = true;

	bool byes_to_strongest = true;
	tournament_matchup_scheme matchup_scheme = tournament_matchup_scheme::STRONGEST_VS_WEAKEST;

	std::vector<std::vector<std::string>> teams;
	std::vector<std::string> maps;
	// END GEN INTROSPECTOR

	static tournament_config from_file(const augs::path_type& path);
};
