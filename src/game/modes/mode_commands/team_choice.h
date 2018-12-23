#pragma once
#include "game/enums/faction_type.h"

namespace mode_commands {
	struct team_choice {
		// GEN INTROSPECTOR struct mode_commands::team_choice
		faction_type target_team = faction_type::COUNT;
		// END GEN INTROSPECTOR

		bool is_set() const {
			return target_team != faction_type::COUNT;
		}

		bool operator==(const team_choice& b) const {
			return target_team == b.target_team;
		}
	};
}
