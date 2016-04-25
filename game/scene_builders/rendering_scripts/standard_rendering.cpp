#include "all.h"

#include "augs/entity_system/entity.h"
#include "augs/entity_system/world.h"

#include "game/systems/render_system.h"
#include "game/systems/gui_system.h"
#include "game/resources/manager.h"

#include "math/matrix.h"

namespace rendering_scripts {
	void standard_rendering(messages::camera_render_request_message msg) {
		auto state = msg.state;
		auto mask = msg.mask;
		auto camera = msg.camera;

		auto& world = camera->get_owner_world();
		auto& render = world.get_system<render_system>();
		auto& gui = world.get_system<gui_system>();

		auto matrix = augs::orthographic_projection<float>(0, state.visible_world_area.x, state.visible_world_area.y, 0, 0, 1);

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);
		auto& default_highlight_shader = *resource_manager.find(assets::program_id::DEFAULT_HIGHLIGHT);
		auto& circular_bars_shader = *resource_manager.find(assets::program_id::CIRCULAR_BARS);
		
		default_shader.use();
		{
			auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render.generate_layers_from_visible_entities(mask);

		for (int i = render_layer::UNDER_GROUND; i > render_layer::DYNAMIC_BODY; --i) {
			render.draw_layer(state, i);
		}

		state.output->call_triangles();
		state.output->clear_triangles();

		default_highlight_shader.use();
		{
			auto projection_matrix_uniform = glGetUniformLocation(default_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render.draw_layer(state, render_layer::DYNAMIC_BODY, true);

		state.output->call_triangles();
		state.output->clear_triangles();

		default_shader.use();

		for (int i = render_layer::DYNAMIC_BODY; i >= 0; --i) {
			render.draw_layer(state, i);
		}

		state.output->call_triangles();
		state.output->clear_triangles();

		state.output->draw_debug_info(
			msg.state.visible_world_area, 
			msg.state.camera_transform, 
			assets::texture_id::BLANK, 
			render.targets, 
			render.view_interpolation_ratio());

		circular_bars_shader.use();
		{
			auto projection_matrix_uniform = glGetUniformLocation(circular_bars_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
			
			vec2 upper(0.0f, 0.0f);
			vec2 lower(1.0f, 1.0f);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(upper);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(lower);
			auto center = (upper + lower) / 2;

			glUniform2f(glGetUniformLocation(circular_bars_shader.id, "texture_center"), center.x, center.y);
		}

		gui.hud.draw_circular_bars(msg);

		state.output->call_triangles();
		state.output->clear_triangles();
		
		default_shader.use();

		gui.hud.draw_circular_bars_information(msg);

		gui.draw_complete_gui_for_camera_rendering_request(msg);

		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		state.output->call_triangles();
		state.output->clear_triangles();
	}
}