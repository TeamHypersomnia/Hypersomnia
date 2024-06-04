#pragma once
#include <variant>
#include "game/enums/faction_choice_result.h"
#include "game/modes/mode_player_id.h"

namespace messages {
	enum class no_arg_mode_notification {
		TEAMS_ARE_NOT_VIABLE_FOR_RANKED,
		FAILED_TO_AUTHENTICATE,
		FAILED_TO_CHECK_BANS,

		PLAYER_READY_FOR_RANKED,
		PLAYER_REQUESTED_ABANDON,

		RANKED_STARTING,
		RANKED_STARTED,

		SHORT_MATCH
	};

	enum class joined_or_left {
		JOINED,
		LEFT,
		RANKED_BANNED
	};

	struct faction_choice {
		faction_choice_result result = faction_choice_result::FAILED;
		faction_type target_faction = faction_type::DEFAULT;

		bool operator==(const faction_choice& b) const = default;
	};

	struct mode_notification {
		mode_player_id subject_mode_id;
		std::string subject_name;
		std::string subject_account_id;
		int players_left = 0;
		std::variant<faction_choice, joined_or_left, no_arg_mode_notification> payload;
	};
}
