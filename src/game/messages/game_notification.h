#pragma once
#include <variant>
#include "game/enums/faction_choice_result.h"
#include "game/modes/mode_player_id.h"

namespace messages {
	struct teleportation {
		entity_id teleported;
		entity_id to_portal;
	};

	struct game_notification {
		std::variant<teleportation> payload;
	};
}
