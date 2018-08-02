#include "application/main/draw_editor_elements.h"
#include "application/setups/editor/editor_setup.h"
#include "view/rendering_scripts/draw_marker_borders.h"

void draw_editor_elements(
	const editor_setup& editor,
	const visible_entities& all_visible,
	const camera_cone cone,
	const augs::drawer_with_default& drawer,
	const augs::line_drawer_with_default& line_drawer,
	const editor_settings& editor_cfg,
	const necessary_images_in_atlas_map& necessary_images,
	const vec2i mouse_pos,
	const vec2i screen_size
) {
	auto on_screen = [cone](const auto p) {
		return cone.to_screen_space(p);
	};

	editor.for_each_icon(
		all_visible,
		[&](const auto typed_handle, const auto image_id, const transformr world_transform, const rgba color) {
			const auto screen_space = transformr(vec2i(on_screen(world_transform.pos)), world_transform.rotation);

			const auto image_size = necessary_images[image_id].get_original_size();

			const auto blank_tex = drawer.default_texture;

			if (auto active_color = editor.find_highlight_color_of(typed_handle.get_id())) {
				active_color->a = static_cast<rgba_channel>(std::min(1.8 * active_color->a, 255.0));

				augs::detail_sprite(
					drawer.output_buffer,
					blank_tex,
					image_size + vec2i(10, 10),
					screen_space.pos,
					screen_space.rotation,
					*active_color
				);

				active_color->a = static_cast<rgba_channel>(std::min(2.2 * active_color->a, 255.0));

				line_drawer.border(
					image_size,
					screen_space.pos,
					screen_space.rotation,
					*active_color,
					border_input { 1, 0 }
				);
			}

			augs::detail_sprite(
				drawer.output_buffer,
				necessary_images.at(image_id),
				screen_space.pos,
				screen_space.rotation,
				color
			);

			line_drawer.border(
				image_size,
				screen_space.pos,
				screen_space.rotation,
				color,
				border_input { 1, 2 }
			);

			::draw_marker_borders(typed_handle, line_drawer, screen_space, cone.eye.zoom, 1.f, color);
		}	
	);

	if (auto eye = editor.find_current_camera_eye()) {
		eye->transform.pos.discard_fract();

		if (const auto view = editor.find_view()) {
			if (view->show_grid) {
				drawer.grid(
					screen_size,
					view->grid.unit_pixels,
					*eye,
					editor_cfg.grid.render
				);
			}
		}

		if (const auto selection_aabb = editor.find_selection_aabb()) {
			auto col = white;

			if (editor.is_mover_active()) {
				col.a = 120;
			}

			drawer.border(
				camera_cone(*eye, screen_size).to_screen_space(*selection_aabb),
				col,
				border_input { 1, -1 }
			);
		}
	}

	editor.for_each_dashed_line(
		[&](vec2 from, vec2 to, const rgba color, const double secs = 0.0, bool fatten = false) {
			const auto a = on_screen(from.round_fract());
			const auto b = on_screen(to.round_fract());

			line_drawer.dashed_line(a, b, color, 5.f, 5.f, secs);

			if (fatten) {
				const auto ba = b - a;
				const auto perp = ba.perpendicular_cw().normalize();
				line_drawer.dashed_line(a + perp, b + perp, color, 5.f, 5.f, secs);
				line_drawer.dashed_line(a + perp * 2, b + perp * 2, color, 5.f, 5.f, secs);
			}
		}	
	);

	if (const auto r = editor.find_screen_space_rect_selection(screen_size, mouse_pos)) {
		drawer.aabb_with_border(
			*r,
			editor_cfg.rectangular_selection_color,
			editor_cfg.rectangular_selection_border_color
		);
	}
}
