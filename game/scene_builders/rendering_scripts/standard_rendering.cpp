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

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);

		default_shader.use();
		
		auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");

		glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, 
			augs::orthographic_projection<float>(0, state.visible_world_area.x, state.visible_world_area.y, 0, 0, 1).data());

		render.draw_all_visible_entities(state, mask);

		state.output->call_triangles();
		state.output->clear_triangles();

		state.output->draw_debug_info(
			msg.state.visible_world_area, 
			msg.state.camera_transform, 
			assets::texture_id::BLANK, 
			render.targets, 
			render.view_interpolation_ratio());

		gui.draw_complete_gui_for_camera_rendering_request(msg);

		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		state.output->call_triangles();
		state.output->clear_triangles();
	}
}