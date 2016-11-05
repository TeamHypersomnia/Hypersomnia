#include <array>
#include "light_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/light_component.h"
#include "game/components/polygon_component.h"

#include "game/resources/manager.h"
#include "game/messages/visibility_information.h"

#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/render_system.h"

#include "game/enums/filters.h"

void light_system::construct(const const_entity_handle) {

}

void light_system::destruct(const const_entity_handle) {

}

void light_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void light_system::render_all_lights(augs::renderer& output, const std::array<float, 16> projection_matrix, viewing_step& step) {
	const auto& cosmos = step.cosm;

	ensure_eq(0, output.get_triangle_count());

	output.light_fbo.use();
	output.clear_current_fbo();

	auto& light_program = *resource_manager.find(assets::program_id::LIGHT);
	light_program.use();

	const auto light_pos_uniform = glGetUniformLocation(light_program.id, "light_pos");
	const auto light_max_distance_uniform = glGetUniformLocation(light_program.id, "max_distance");
	const auto light_attenuation_uniform = glGetUniformLocation(light_program.id, "light_attenuation");
	const auto light_multiply_color_uniform = glGetUniformLocation(light_program.id, "multiply_color");
	const auto projection_matrix_uniform = glGetUniformLocation(light_program.id, "projection_matrix");

	std::vector<messages::visibility_information_request> requests;
	std::vector<messages::visibility_information_response> responses;

	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projection_matrix.data());

	for (const auto it : cosmos.get(processing_subjects::WITH_LIGHT)) {
		messages::visibility_information_request request;
		request.eye_transform = it.viewing_transform();
		request.filter = filters::line_of_sight_query();
		request.square_side = 1500;
		request.subject = it;

		requests.push_back(request);
	}

	{
		std::vector<messages::line_of_sight_response> dummy;
		visibility_system().respond_to_visibility_information_requests(cosmos, {}, requests, dummy, responses);
	}

	const auto camera_transform = step.camera_state.camera_transform;
	const auto camera_size = step.camera_state.visible_world_area;

	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosmos[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		auto& cache = per_entity_cache[light_entity.get_id().pool.indirection_index];

		for (size_t t = 0; t < r.get_num_triangles(); ++t) {
			const auto world_light_tri = r.get_world_triangle(t, requests[i].eye_transform.pos);
			vertex_triangle renderable_light_tri;

			renderable_light_tri.vertices[0].pos = world_light_tri.points[0] - camera_transform.pos + camera_size/2;
			renderable_light_tri.vertices[1].pos = world_light_tri.points[1] - camera_transform.pos + camera_size/2;
			renderable_light_tri.vertices[2].pos = world_light_tri.points[2] - camera_transform.pos + camera_size/2;

			renderable_light_tri.vertices[0].color = light.color;
			renderable_light_tri.vertices[1].color = light.color;
			renderable_light_tri.vertices[2].color = light.color;

			output.push_triangle(renderable_light_tri);
		}

		auto screen_pos = requests[i].eye_transform - camera_transform;
		screen_pos.pos.x += camera_size.x * 0.5f;
		screen_pos.pos.y = camera_size.y - (screen_pos.pos.y + camera_size.y * 0.5f);

		glUniform2f(light_pos_uniform, screen_pos.pos.x, screen_pos.pos.y);

		glUniform1f(light_max_distance_uniform, light.max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			light.constant.base_value,
			light.linear.base_value,
			light.quadratic.base_value
		);

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);

		output.call_triangles();
		output.clear_triangles();

		glUniform1f(light_max_distance_uniform, light.wall_max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			light.wall_constant.base_value,
			light.wall_linear.base_value,
			light.wall_quadratic.base_value
		);
		
		glUniform3f(light_multiply_color_uniform,
			light.color.r/255.f,
			light.color.g/255.f,
			light.color.b/255.f);
		
		render_system().draw_entities(output.triangles, step.visible_per_layer[render_layer::DYNAMIC_BODY], step.camera_state);

		output.call_triangles();
		output.clear_triangles();

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);
	}

	graphics::fbo::use_default();

	output.set_active_texture(2);
	output.bind_texture(output.light_fbo);
	output.set_active_texture(0);
}