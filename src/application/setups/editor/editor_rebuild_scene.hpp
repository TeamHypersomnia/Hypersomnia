#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

#include "application/arena/arena_handle.h"
#include "application/arena/build_arena_from_editor_project.h"

void editor_setup::rebuild_arena(const bool editor_preview) {
	const bool for_playtesting = true;
	const auto override_game_mode = game_mode_name_type("");

	::build_arena_from_editor_project<editor_arena_handle<false>>(
		get_arena_handle(),
		{
			project,
			override_game_mode,
			paths.project_folder,
			official,
			std::addressof(scene_entity_to_node),
			std::addressof(clean_round_state),
			for_playtesting,
			editor_preview
		}
	);

	inspected_to_entity_selector_state();
}
