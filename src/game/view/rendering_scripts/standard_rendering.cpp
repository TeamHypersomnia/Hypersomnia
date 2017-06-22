#include "all.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"

#include "game/components/render_component.h"

#include "game/detail/gui/character_gui.h"
#include "game/systems_inferred/tree_of_npo_system.h"
#include "game/assets/assets_manager.h"
#include "augs/graphics/renderer.h"
#include "game/view/viewing_step.h"

#include "augs/math/matrix.h"

#include "augs/graphics/OpenGL_includes.h"

namespace rendering_scripts {
	void standard_rendering(const viewing_step step) {
		const auto& camera = step.camera;
		auto& renderer = step.renderer;
		auto& output = renderer.triangles;
		const auto& cosmos = step.cosm;
		const auto& controlled_entity = cosmos[step.viewed_character];
		const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
		const auto global_time_seconds = (step.get_interpolated_total_time_passed_in_seconds());

		const auto& visible_per_layer = step.visible.per_layer;

		const auto matrix = augs::orthographic_projection<float>(0, camera.visible_world_area.x, camera.visible_world_area.y, 0, 0, 1);

		const auto& manager = get_assets_manager();

		auto& default_shader = manager[assets::program_id::DEFAULT];
		auto& pure_color_highlight_shader = manager[assets::program_id::PURE_COLOR_HIGHLIGHT];
		auto& border_highlight_shader = pure_color_highlight_shader; // the same
		auto& circular_bars_shader = manager[assets::program_id::CIRCULAR_BARS];
		
		default_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		for (int i = static_cast<int>(render_layer::UNDER_GROUND); i > static_cast<int>(render_layer::DYNAMIC_BODY); --i) {
			render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[static_cast<render_layer>(i)], camera, renderable_drawing_type::NORMAL);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		border_highlight_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(border_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], camera, renderable_drawing_type::BORDER_HIGHLIGHTS);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		for (int i = int(render_layer::DYNAMIC_BODY); i >= 0; --i) {
			render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer(i)], camera, renderable_drawing_type::NORMAL);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		circular_bars_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(circular_bars_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
			
			vec2 upper(0.0f, 0.0f);
			vec2 lower(1.0f, 1.0f);
			upper = manager[assets::game_image_id::CIRCULAR_BAR_MEDIUM].texture_maps[texture_map_type::DIFFUSE].get_atlas_space_uv(upper);
			lower = manager[assets::game_image_id::CIRCULAR_BAR_MEDIUM].texture_maps[texture_map_type::DIFFUSE].get_atlas_space_uv(lower);
			const auto center = (upper + lower) / 2;
		
			glUniform2f(glGetUniformLocation(circular_bars_shader.id, "texture_center"), center.x, center.y);
		
		}

		const auto& textual_infos = draw_circular_bars_and_get_textual_info(step);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		renderer.call_triangles(textual_infos);

		pure_color_highlight_shader.use();

		// hud.draw_pure_color_highlights(step);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		// hud.draw_vertically_flying_numbers(step);

		renderer.bind_texture(manager[assets::gl_texture_id::GAME_WORLD_ATLAS]);

		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.draw_debug_info(
			camera,
			assets::game_image_id::BLANK,
			{},
			step.get_interpolation_ratio()
		);
	}
}