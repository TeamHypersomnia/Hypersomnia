#pragma once
#include <variant>
#include "augs/gui/formatted_string.h"
#include "game/modes/mode_player_id.h"

namespace messages {
	struct two_player_message {
		std::string first_name;
		std::string second_name;

		faction_type first_faction = faction_type::SPECTATOR;
		faction_type second_faction = faction_type::SPECTATOR;

		std::string preffix;
		std::string mid;
		std::string suffix;

		bool bbcode = false;
	};

	enum class special_hud_command {
		CLEAR
	};

	using message_variant = std::variant<special_hud_command, augs::gui::text::formatted_string, two_player_message>;

	struct hud_message {
		message_variant payload;
	};

	struct team_match_start_message {
		struct player_entry {
			std::string nickname;
		};

		std::vector<player_entry> team_1;
		std::vector<player_entry> team_2;
	};

	struct duel_of_honor_message {
		std::string first_player;
		std::string second_player;
	};

	struct duel_interrupted_message {
		int deserter_score = 0;
		int opponent_score = 0;

		std::string deserter_nickname;
		std::string opponent_nickname;

		bool is_truce() const {
			return deserter_score >= opponent_score;
		}

		bool was_winning() const {
			return deserter_score > opponent_score;
		}
	};

	struct match_summary_ended {
		bool is_final = false;
	};

	struct match_summary_message {
		struct player_entry {
			mode_player_id id;

			int kills = 0;
			int assists = 0;
			int deaths = 0;

			int score = 0;

			std::string nickname;
		};

		int first_team_score = 0;
		int second_team_score = 0;

		std::vector<player_entry> first_faction;
		std::vector<player_entry> second_faction;

		mode_player_id mvp_player_id;

		bool is_tie() const {
			return first_team_score == second_team_score;
		}

		void flip_teams() {
			std::swap(first_team_score, second_team_score);
			std::swap(first_faction, second_faction);
		}
	};
}
