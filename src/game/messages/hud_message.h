#pragma once
#include <variant>
#include "augs/gui/formatted_string.h"

namespace messages {
	struct two_player_message {
		std::string first_name;
		std::string second_name;

		faction_type first_faction = faction_type::COUNT;
		faction_type second_faction = faction_type::COUNT;

		std::string preffix;
		std::string mid;
		std::string suffix;

		bool bbcode = false;
	};

	using message_variant = std::variant<augs::gui::text::formatted_string, two_player_message>;

	struct hud_message {
		message_variant payload;
	};
}
