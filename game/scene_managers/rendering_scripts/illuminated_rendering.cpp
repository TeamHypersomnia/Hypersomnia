#include "all.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"

#include "game/components/gui_element_component.h"
#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/resources/manager.h"
#include "augs/graphics/renderer.h"
#include "game/transcendental/step.h"
#include "game/transcendental/viewing_session.h"

#include "game/detail/gui/immediate_hud.h"

#include "math/matrix.h"

#include "3rdparty/GL/OpenGL.h"

namespace rendering_scripts {
	void illuminated_rendering(viewing_step& step) {
		const auto& state = step.camera_state;
		auto& renderer = step.renderer;
		auto& output = renderer.triangles;
		const auto& cosmos = step.cosm;
		const auto& dynamic_tree = cosmos.systems_temporary.get<dynamic_tree_system>();
		const auto& physics = cosmos.systems_temporary.get<physics_system>();
		const auto& controlled_entity = cosmos[step.camera_state.associated_character];
		const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
		const auto& particles = step.session.systems_audiovisual.get<particles_simulation_system>();
		const float global_time_seconds = step.get_interpolated_total_time_passed_in_seconds();

		step.visible_entities = cosmos[dynamic_tree.determine_visible_entities_from_camera(state, physics)];
		step.visible_per_layer = render_system().get_visible_per_layer(step.visible_entities);

		const auto matrix = augs::orthographic_projection<float>(0, state.camera.visible_world_area.x, state.camera.visible_world_area.y, 0, 0, 1);

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);
		auto& illuminated_shader = *resource_manager.find(assets::program_id::DEFAULT_ILLUMINATED);
		auto& pure_color_highlight_shader = *resource_manager.find(assets::program_id::PURE_COLOR_HIGHLIGHT);
		auto& border_highlight_shader = pure_color_highlight_shader; // the same
		auto& circular_bars_shader = *resource_manager.find(assets::program_id::CIRCULAR_BARS);
		auto& smoke_shader = *resource_manager.find(assets::program_id::SMOKE);

		particles_simulation_system::drawing_input particles_input(output);
		particles_input.camera = state.camera;

		default_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		renderer.smoke_fbo.use();
		glClear(GL_COLOR_BUFFER_BIT);

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE); glerr;

		particles.draw(render_layer::DIM_SMOKES, particles_input);

		renderer.call_triangles();
		renderer.clear_triangles();

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;

		renderer.smoke_fbo.use_default();

		auto& light = step.session.systems_audiovisual.get<light_system>();

		light.render_all_lights(renderer, matrix, step);

		illuminated_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(illuminated_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		for (int i = render_layer::UNDER_GROUND; i > render_layer::DYNAMIC_BODY; --i) {
			render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[i], state, renderable_drawing_type::NORMAL);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		border_highlight_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(border_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], state, renderable_drawing_type::BORDER_HIGHLIGHTS);
		
		renderer.call_triangles();
		renderer.clear_triangles();
		
		illuminated_shader.use();
		
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::DYNAMIC_BODY], state, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], state, renderable_drawing_type::NORMAL);
		
		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.set_active_texture(1);
		renderer.bind_texture(renderer.smoke_fbo);
		renderer.set_active_texture(0);

		smoke_shader.use();

		renderer.fullscreen_quad();

		default_shader.use();
		
		for (int i = render_layer::FLYING_BULLETS; i >= 0; --i) {
			render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[i], state, renderable_drawing_type::NORMAL);
		}
		
		particles.draw(render_layer::EFFECTS, particles_input);

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

		const auto& hud = step.session.hud;

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

		renderer.bind_texture(*resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS));

		renderer.call_triangles();
		renderer.clear_triangles();

		//renderer.draw_debug_info(
		//	state.visible_world_area,
		//	state.camera_transform,
		//	assets::texture_id::BLANK,
		//	{},
		//	step.get_delta().view_interpolation_ratio());
	}
}