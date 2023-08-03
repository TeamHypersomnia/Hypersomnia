#pragma once
#include <variant>
#include "game/enums/faction_choice_result.h"
#include "game/modes/mode_player_id.h"

namespace messages {
	enum class joined_or_left {
		JOINED,
		LEFT
	};

	struct faction_choice {
		faction_choice_result result = faction_choice_result::FAILED;
		faction_type target_faction = faction_type::DEFAULT;
	};

	struct mode_notification {
		mode_player_id subject_mode_id;
		std::string subject_name;
		std::variant<faction_choice, joined_or_left> payload;
	};
}
