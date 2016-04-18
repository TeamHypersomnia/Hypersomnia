#include "all.h"

#include "augs/entity_system/entity.h"
#include "augs/entity_system/world.h"

#include "game_framework/systems/render_system.h"
#include "game_framework/resources/manager.h"

#include "math/matrix.h"

namespace rendering_scripts {
	void standard_rendering(messages::camera_render_request_message msg) {
		auto state = msg.state;
		auto mask = msg.mask;
		auto camera = msg.camera;

		auto& world = camera->get_owner_world();
		auto& drawing = world.get_system<render_system>();

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);

		default_shader.use();
		
		auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");

		glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, 
			augs::orthographic_projection<float>(0, state.visible_world_area.x, state.visible_world_area.y, 0, 0, 1).data());

		drawing.draw_all_visible_entities(state, mask);

		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		state.output->call_triangles();
		state.output->clear_triangles();
	}
}