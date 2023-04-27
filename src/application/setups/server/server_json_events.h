#pragma once

namespace server_json_events {
	struct match_start_player {
		// GEN INTROSPECTOR struct server_json_events::match_start_player
		std::string nickname;
		// END GEN INTROSPECTOR
	};

	struct match_end_player {
		// GEN INTROSPECTOR struct server_json_events::match_end_player
		int score = 0;
		std::string nickname;
		// END GEN INTROSPECTOR
	};

	struct match_end_team {
		// GEN INTROSPECTOR struct server_json_events::match_end_team
		int score = 0;

		std::vector<match_end_player> players;
		// END GEN INTROSPECTOR
	};

	struct match_start_team {
		// GEN INTROSPECTOR struct server_json_events::match_start_team
		std::vector<match_start_player> players;
		// END GEN INTROSPECTOR
	};

	struct match_start {
		// GEN INTROSPECTOR struct server_json_events::match_start
		match_start_team team_1;
		match_start_team team_2;
		// END GEN INTROSPECTOR
	};

	struct match_end {
		// GEN INTROSPECTOR struct server_json_events::match_end
		match_end_team team_1;
		match_end_team team_2;
		// END GEN INTROSPECTOR
	};
}

