#include "application/main/draw_debug_lines.h"

#include "game/debug_drawing_settings.h"
#include "game/cosmos/cosmos.h"
#include "augs/math/camera_cone.h"
#include "application/config_lua_table.h"
#include "augs/drawing/drawing.h"

void draw_debug_lines(
	const cosmos& viewed_cosmos,
	augs::renderer& renderer,
	const real32 interpolation_ratio,
	const augs::atlas_entry default_texture,
	const config_lua_table& cfg,
	const camera_cone cone
) {
	if (DEBUG_DRAWING.draw_npo_tree_nodes) {
		(void)viewed_cosmos;
#if TODO
		viewed_cosmos.get_solvable_inferred().tree_of_npo.for_each_aabb([](const ltrb aabb){
			auto& lines = DEBUG_FRAME_LINES;

			lines.emplace_back(red, aabb.left_top(), aabb.right_top());
			lines.emplace_back(red, aabb.right_top(), aabb.right_bottom());
			lines.emplace_back(red, aabb.right_bottom(), aabb.left_bottom());
			lines.emplace_back(red, aabb.left_bottom(), aabb.left_top());
		});
#endif
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

	const auto& query_mult = cfg.session.camera_query_aabb_mult;

	if (DEBUG_DRAWING.draw_camera_query) {
		if (!augs::compare(query_mult, 1.f)) {
			auto multiplied_cone = cone;
			auto& zoom = multiplied_cone.eye.zoom;
			zoom /= query_mult;

			augs::drawer_with_default(renderer.get_triangle_buffer(), default_texture).border(
				multiplied_cone.get_visible_world_rect_aabb(),
				white,
				border_input { static_cast<int>(std::lround(3.f / zoom)), 0 }
			);

			renderer.call_and_clear_triangles();
		}
	}
}

void draw_headshot_debug_lines(
	vec2 missile_pos,
	vec2 impact_dir,
	vec2 head_pos,
	float head_radius
) {
	if (DEBUG_DRAWING.draw_headshot_detection) {
		DEBUG_PERSISTENT_LINES.emplace_back(
			cyan,
			missile_pos,
			missile_pos + impact_dir * 100
		);

		DEBUG_PERSISTENT_LINES.emplace_back(
			red,
			head_pos,
			head_pos + vec2(0, head_radius)
		);

		DEBUG_PERSISTENT_LINES.emplace_back(
			red,
			head_pos,
			head_pos + vec2(head_radius, 0)
		);

		DEBUG_PERSISTENT_LINES.emplace_back(
			red,
			head_pos,
			head_pos + vec2(-head_radius, 0)
		);

		DEBUG_PERSISTENT_LINES.emplace_back(
			red,
			head_pos,
			head_pos + vec2(0, -head_radius)
		);
	}
}
