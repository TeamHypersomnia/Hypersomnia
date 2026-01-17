#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/item_component.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/enums/slot_function.h"
#include "game/detail/inventory/wielding_setup.h"

/*
	Find the most expensive weapon in the bot's inventory.
	Returns the item id if found, otherwise returns dead entity_id.
*/

template <class E>
inline entity_id find_best_weapon(const E& character_handle) {
	entity_id best_weapon;
	money_type best_price = 0;

	character_handle.for_each_contained_item_recursive(
		[&](const auto& item) {
			if (const auto item_def = item.template find<invariants::item>()) {
				if (item_def->categories_for_slot_compatibility.test(item_category::HAND_HOLDABLE_MAGAZINE)) {
					return;
				}

				if (item_def->categories_for_slot_compatibility.test(item_category::MAGAZINE)) {
					return;
				}

				const auto price = item_def->standard_price;

				if (price > best_price) {
					best_price = price;
					best_weapon = item.get_id();
				}
			}
		}
	);

	return best_weapon;
}

/*
	Determines if the bot should holster weapons.
	Returns true when patrolling/traveling to first waypoint without combat.
	Always false in COMBAT.
*/

inline bool should_holster_weapons(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state == bot_state_type::COMBAT) {
		return false;
	}

	if (ai_state.current_state == bot_state_type::PATROLLING) {
		if (ai_state.going_to_first_waypoint) {
			return true;
		}
	}

	if (ai_state.current_state == bot_state_type::PUSHING) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PLANTING) {
		return true;
	}

	return false;
}

/*
	Determines if the bot should sprint.
	- First/switching waypoint in patrol
	- Going to PUSH waypoint
	- In COMBAT (chasing)
*/

inline bool should_sprint(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state == bot_state_type::COMBAT) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PATROLLING && ai_state.going_to_first_waypoint) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PUSHING) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::RETRIEVING_BOMB) {
		return true;
	}

	return false;
}

/*
	Determines if the bot should walk silently.
	Only when patrolling (and not going to first waypoint).
	85% chance to walk silently when choosing next waypoint.
*/

inline bool should_walk_silently(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state != bot_state_type::PATROLLING) {
		return false;
	}

	if (ai_state.going_to_first_waypoint) {
		return false;
	}

	return ai_state.walk_silently_to_next_waypoint;
}

/*
	Check if a position is within line of sight from another position.
	Uses the physics system for raycasting.
*/

template <class E>
inline bool is_in_line_of_sight(
	const vec2 from_pos,
	const vec2 to_pos,
	const physics_world_cache& physics,
	const cosmos& cosm,
	const E& ignore_entity
) {
	const auto filter = predefined_queries::line_of_sight();
	const auto raycast = physics.ray_cast_px(
		cosm.get_si(),
		from_pos,
		to_pos,
		filter,
		ignore_entity
	);

	return !raycast.hit;
}

/*
	Check if a position is within the bot's field of view.
	Normal FOV: 1920x1080
	Extended FOV (camping): 2688x1560
*/

inline bool is_within_fov(
	const vec2 from_pos,
	const vec2 target_pos,
	const bool camping
) {
	const auto offset = target_pos - from_pos;
	const float fov_x = camping ? 2688.0f : 1920.0f;
	const float fov_y = camping ? 1560.0f : 1080.0f;

	return repro::fabs(offset.x) < fov_x && repro::fabs(offset.y) < fov_y;
}

/*
	Check if a waypoint is applicable for a given faction.
	A waypoint with faction DEFAULT applies to both factions.
	Otherwise it only applies if factions match.
*/

inline bool is_waypoint_for_faction(
	const faction_type waypoint_faction,
	const faction_type bot_faction
) {
	if (waypoint_faction == faction_type::DEFAULT) {
		return true;
	}

	return waypoint_faction == bot_faction;
}
