#include "all.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"

#include "game/systems_insignificant/light_system.h"

#include "game/components/gui_element_component.h"
#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/resources/manager.h"
#include "augs/graphics/renderer.h"
#include "game/transcendental/step.h"

#include "game/detail/gui/immediate_hud.h"

#include "math/matrix.h"

#include "3rdparty/GL/OpenGL.h"

namespace rendering_scripts {
	void standard_rendering(viewing_step& step) {
		const auto& state = step.camera_state;
		auto& renderer = step.renderer;
		auto& output = renderer.triangles;
		const auto& cosmos = step.cosm;
		const auto& dynamic_tree = cosmos.systems_temporary.get<dynamic_tree_system>();
		const auto& physics = cosmos.systems_temporary.get<physics_system>();
		const auto& controlled_entity = cosmos[step.camera_state.associated_character];

		step.visible_entities = cosmos[dynamic_tree.determine_visible_entities_from_camera(state, physics)];
		step.visible_per_layer = render_system().get_visible_per_layer(step.visible_entities);

		const auto matrix = augs::orthographic_projection<float>(0, state.visible_world_area.x, state.visible_world_area.y, 0, 0, 1);

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);
		auto& pure_color_highlight_shader = *resource_manager.find(assets::program_id::PURE_COLOR_HIGHLIGHT);
		auto& circular_bars_shader = *resource_manager.find(assets::program_id::CIRCULAR_BARS);
		
		auto& light = cosmos.systems_insignificant.get<light_system>();

		light.render_all_lights(step);

		default_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		for (int i = render_layer::UNDER_GROUND; i > render_layer::DYNAMIC_BODY; --i) {
			render_system().draw_entities(output, step.visible_per_layer[i], state);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		pure_color_highlight_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(pure_color_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render_system().draw_entities(output, step.visible_per_layer[render_layer::DYNAMIC_BODY], state, true);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		for (int i = render_layer::DYNAMIC_BODY; i >= 0; --i) {
			render_system().draw_entities(output, step.visible_per_layer[i], state);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		circular_bars_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(circular_bars_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
			
			vec2 upper(0.0f, 0.0f);
			vec2 lower(1.0f, 1.0f);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(upper);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(lower);
			const auto center = (upper + lower) / 2;
		
			glUniform2f(glGetUniformLocation(circular_bars_shader.id, "texture_center"), center.x, center.y);
		
		}

		//for (const auto it : cosmos.get(processing_subjects::WITH_INPUT_RECEIVER)) {
		//	if (it.get<components::input_receiver>().local && it.has<components::gui_element>()) {
				//auto& gui = it.get<components::gui_element>();
				const auto& hud = step.hud;

				const auto& textual_infos = hud.draw_circular_bars_and_get_textual_info(step);
		
				renderer.call_triangles();
				renderer.clear_triangles();
		
				default_shader.use();
		
				renderer.call_triangles(textual_infos);
		
				pure_color_highlight_shader.use();
		
				hud.draw_pure_color_highlights(step);
		
				renderer.call_triangles();
				renderer.clear_triangles();
		
				default_shader.use();
		
				hud.draw_vertically_flying_numbers(step);
		
				if (controlled_entity.has<components::gui_element>()) {
					components::gui_element::draw_complete_gui_for_camera_rendering_request(output, controlled_entity, step);
				}
				//gui.draw_complete_gui_for_camera_rendering_request(step);
		//	}
		//}

		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.draw_debug_info(
			state.visible_world_area,
			state.camera_transform,
			assets::texture_id::BLANK,
			{},
			step.get_delta().view_interpolation_ratio());
	}
}