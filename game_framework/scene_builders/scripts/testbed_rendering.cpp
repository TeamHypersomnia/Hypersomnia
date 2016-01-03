#include "rendering_procedures.h"

#include "utilities/entity_system/entity.h"
#include "utilities/entity_system/world.h"

#include "game_framework/systems/render_system.h"
#include "game_framework/resources/manager.h"

#include "math/matrix.h"

namespace scripts {
	void testbed_rendering(augs::entity_id camera, shared::drawing_state state, int mask) {
		auto& world = camera->get_owner_world();
		auto& drawing = world.get_system<render_system>();

		auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);

		default_shader.use();
		
		auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");

		glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, 
			augs::orthographic_projection<float>(0, state.visible_area.x, state.visible_area.y, 0, 0, 1).data());

		drawing.generate_and_draw_all_layers(state, mask);

		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		state.output->call_triangles();
		state.output->clear_triangles();
	}
}