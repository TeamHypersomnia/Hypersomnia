#pragma once
#include <optional>
#include "game/cosmos/entity_id.h"
#include "game/enums/faction_type.h"

namespace mode_messages {
	struct add_player {
		std::optional<faction_type> associated_faction;
	};

	struct added_player {
		entity_guid token;
	};
}
