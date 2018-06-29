#pragma once
#include "augs/graphics/renderer.h"
#include "augs/texture_atlas/atlas_entry.h"

#include "game/debug_drawing_settings.h"
#include "game/transcendental/cosmos.h"

inline void draw_debug_lines(
	const cosmos& viewed_cosmos,
	augs::renderer& renderer,
	const real32 interpolation_ratio,
	const augs::atlas_entry default_texture
) {
	if (DEBUG_DRAWING.draw_npo_tree_nodes) {
		viewed_cosmos.get_solvable_inferred().tree_of_npo.for_each_aabb([](const ltrb aabb){
			auto& lines = DEBUG_FRAME_LINES;

			lines.emplace_back(red, aabb.left_top(), aabb.right_top());
			lines.emplace_back(red, aabb.right_top(), aabb.right_bottom());
			lines.emplace_back(red, aabb.right_bottom(), aabb.left_bottom());
			lines.emplace_back(red, aabb.left_bottom(), aabb.left_top());
		});
	}

	renderer.draw_debug_lines(
		DEBUG_LOGIC_STEP_LINES,
		DEBUG_PERSISTENT_LINES,
		DEBUG_FRAME_LINES,

		default_texture,
		interpolation_ratio
	);

	renderer.call_and_clear_lines();

	DEBUG_FRAME_LINES.clear();
}
