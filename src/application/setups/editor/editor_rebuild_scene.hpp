#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"
#include "game/cosmos/change_common_significant.hpp"

#include "application/arena/arena_handle.h"
#include "application/arena/build_arena_from_editor_project.hpp"

void editor_setup::rebuild_scene() {
	::build_arena_from_editor_project(
		get_arena_handle(),
		project,
		[&](const auto& path) { return resolve_project_path(path); },
		official_resources,
		built_official_content,
		std::addressof(scene_entity_to_node),
		nullptr,
		cosmos_common_significant_access()
	);

	inspected_to_entity_selector_state();
}
