#pragma once
#include "game/organization/special_flavour_id_types.h"
#include "game/detail/inventory/requested_equipment.h"

struct bomb_defusal_rules {
	// GEN INTROSPECTOR struct bomb_defusal_rules
	bool dummy = false;
	// END GEN INTROSPECTOR

	static constexpr bool has_economy() {
		return true;
	}

	static auto get_name() {
		return "Bomb Defusal";
	}
};

struct gun_game_rules {
	// GEN INTROSPECTOR struct gun_game_rules
	bool free_for_all = true;

	per_actual_faction<requested_equipment> basic_eq;
	per_actual_faction<requested_equipment> final_eq;

	std::vector<item_flavour_id> progression;
	bool can_throw_melee_on_final_level = true;
	// END GEN INTROSPECTOR

	int get_final_level() const {
		return static_cast<int>(progression.size());
	}

	static constexpr bool has_economy() {
		return false;
	}

	static auto get_name() {
		return "Gun Game";
	}
};

using all_subrules_variant = std::variant<
	bomb_defusal_rules,
	gun_game_rules
>;
