#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

#include "application/arena/arena_handle.h"
#include "application/arena/build_arena_from_editor_project.h"

void editor_setup::build_arena_from(
	const editor_project& source,
	const bool editor_preview,
	const bool showing_navmesh,
	scene_entity_to_node_map* const entity_to_node
) {
	const bool for_playtesting = true;
	const auto override_game_mode = game_mode_name_type("");

	bot_test_paths.invalidate();

	::build_arena_from_editor_project<editor_arena_handle<false>>(
		get_arena_handle(),
		{
			source,
			override_game_mode,
			paths.project_folder,
			official,
			entity_to_node,
			std::addressof(clean_round_state),
			for_playtesting,
			editor_preview,
			showing_navmesh
		}
	);
}

void editor_setup::rebuild_arena(const bool editor_preview) {
	arena_built_showing_navmesh = view.show_navmesh;

	build_arena_from(project, editor_preview, arena_built_showing_navmesh, std::addressof(scene_entity_to_node));

	inspected_to_entity_selector_state();
}
