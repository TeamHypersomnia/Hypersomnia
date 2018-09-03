#pragma once
#include "game/enums/faction_type.h"

namespace mode_commands {
	struct team_choice {
		// GEN INTROSPECTOR struct mode_commands::team_choice
		faction_type target_team = faction_type::COUNT;
		// END GEN INTROSPECTOR
	};
}
