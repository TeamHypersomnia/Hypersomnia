#pragma once
#include <variant>
#include "augs/gui/formatted_string.h"

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

	struct duel_of_honor_message {
		std::string first_player;
		std::string second_player;
	};
}
