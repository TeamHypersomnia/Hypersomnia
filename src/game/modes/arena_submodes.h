#pragma once
#include "game/organization/special_flavour_id_types.h"

struct bomb_defusal_ruleset {
	// GEN INTROSPECTOR struct bomb_defusal_ruleset
	bool dummy = false;
	// END GEN INTROSPECTOR
};

struct gun_game_ruleset {
	// GEN INTROSPECTOR struct gun_game_ruleset
	requested_equipment final_eq;
	std::vector<item_flavour_id> progression;
	// END GEN INTROSPECTOR
};

using all_subrulesets_variant = std::variant<
	bomb_defusal_ruleset,
	gun_game_ruleset
>;

using all_subrulesets_variant = std::variant<
	bomb_defusal_ruleset,
	gun_game_ruleset
>;
