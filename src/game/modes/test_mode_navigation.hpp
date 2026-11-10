#pragma once
#include <optional>
#include "game/cosmos/cosmos.h"
#include "game/cosmos/make_physics_path_hints.h"
#include "game/components/movement_component.h"
#include "game/detail/pathfinding/pathfinding.h"
#include "game/detail/path_navigation/navigate_path.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/difficulty_type.h"

enum class test_navigation_result {
	NAVIGATING,
	ARRIVED,
	NO_PATH
};

template <class Character>
void stop_test_character(const Character& character) {
	if (auto* movement = character.template find<components::movement>()) {
		movement->flags.set_from_closest_direction(vec2::zero);
		movement->flags.sprinting = false;
	}
}

template <class Character>
test_navigation_result navigate_test_character(
	const cosmos& cosm,
	std::optional<ai_path_navigation_state>& navigation,
	const Character& character,
	const transformr target_transform,
	const bool exact_destination,
	const bool allow_sprint
) {
	const auto character_pos = character.get_logic_transform().pos;
	const auto& navmesh = cosm.get_common_significant().navmesh;
	const auto physics_hints = make_physics_path_hints(cosm);

	if (navigation == std::nullopt) {
		navigation = ::start_navigating_to(
			character_pos,
			target_transform,
			navmesh,
			&physics_hints
		);

		if (navigation == std::nullopt) {
			return test_navigation_result::NO_PATH;
		}
	}

	if (exact_destination && navigation.has_value()) {
		navigation->exact_destination = true;
	}

	const real32 dt_secs = cosm.get_fixed_delta().in_seconds();

	const auto nav_result = ::navigate_path(
		navigation,
		character_pos,
		navmesh,
		character,
		dt_secs,
		&physics_hints
	);

	if (nav_result.is_navigating && nav_result.movement_direction.has_value()) {
		if (auto* movement = character.template find<components::movement>()) {
			movement->flags.set_from_closest_direction(*nav_result.movement_direction);
			movement->flags.sprinting = allow_sprint && nav_result.can_sprint;
		}
	}
	else if (nav_result.path_completed) {
		navigation = std::nullopt;
		::stop_test_character(character);
	}

	const bool still_navigating = navigation.has_value();

	::interpolate_crosshair(
		character.find_crosshair(),
		nav_result.crosshair_offset,
		dt_secs,
		difficulty_type::HARD,
		still_navigating
	);

	return still_navigating ? test_navigation_result::NAVIGATING : test_navigation_result::ARRIVED;
}
