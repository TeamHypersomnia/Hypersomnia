#include <array>
#include "light_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/viewing_session.h"
#include "game/components/light_component.h"
#include "game/components/polygon_component.h"

#include "game/resources/manager.h"
#include "game/messages/visibility_information.h"

#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/render_system.h"

#include "game/enums/filters.h"

void light_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

light_system::cache::cache() {
	std::fill(all_variation_values.begin(), all_variation_values.end(), 0.f);
}

void light_system::render_all_lights(augs::renderer& output, const std::array<float, 16> projection_matrix, viewing_step& step) {
	const auto& cosmos = step.cosm;
	const auto dt = step.get_delta();

	ensure_eq(0, output.get_triangle_count());

	output.light_fbo.use();
	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
	output.clear_current_fbo();
	glClearColor(0.f, 0.f, 0.f, 0.f);

	auto& light_program = *resource_manager.find(assets::program_id::LIGHT);
	auto& default_program = *resource_manager.find(assets::program_id::DEFAULT);
	light_program.use();

	const auto light_pos_uniform = glGetUniformLocation(light_program.id, "light_pos");
	const auto light_max_distance_uniform = glGetUniformLocation(light_program.id, "max_distance");
	const auto light_attenuation_uniform = glGetUniformLocation(light_program.id, "light_attenuation");
	const auto light_multiply_color_uniform = glGetUniformLocation(light_program.id, "multiply_color");
	const auto projection_matrix_uniform = glGetUniformLocation(light_program.id, "projection_matrix");
	const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
	const auto& particles = step.session.systems_audiovisual.get<particles_simulation_system>();

	std::vector<messages::visibility_information_request> requests;
	std::vector<messages::visibility_information_response> responses;

	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projection_matrix.data());

	for (const auto it : cosmos.get(processing_subjects::WITH_LIGHT)) {
		messages::visibility_information_request request;
		request.eye_transform = it.viewing_transform(interp);
		request.filter = filters::line_of_sight_query();
		request.square_side = it.get<components::light>().max_distance.base_value;
		request.subject = it;

		requests.push_back(request);
	}

	{
		std::vector<messages::line_of_sight_response> dummy;
		visibility_system().respond_to_visibility_information_requests(cosmos, {}, requests, dummy, responses);
	}

	const auto camera_transform = step.camera_state.camera.transform;
	const auto camera_size = step.camera_state.camera.visible_world_area;
	const auto camera_offset = -camera_transform.pos + camera_size / 2;

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE); glerr;
	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosmos[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		auto& cache = per_entity_cache[light_entity.get_id().pool.indirection_index];

		const float delta = dt.in_seconds();

		light.constant.variation.update_value(rng, cache.all_variation_values[0], delta);
		light.linear.variation.update_value(rng, cache.all_variation_values[1], delta);
		light.quadratic.variation.update_value(rng, cache.all_variation_values[2], delta);
		
		light.wall_constant.variation.update_value(rng, cache.all_variation_values[3], delta);
		light.wall_linear.variation.update_value(rng, cache.all_variation_values[4], delta);
		light.wall_quadratic.variation.update_value(rng, cache.all_variation_values[5], delta);
		
		light.position_variations[0].update_value(rng, cache.all_variation_values[6], delta);
		light.position_variations[1].update_value(rng, cache.all_variation_values[7], delta);

		for (size_t t = 0; t < r.get_num_triangles(); ++t) {
			const auto world_light_tri = r.get_world_triangle(t, requests[i].eye_transform.pos);
			vertex_triangle renderable_light_tri;

			renderable_light_tri.vertices[0].pos = world_light_tri.points[0] + camera_offset;
			renderable_light_tri.vertices[1].pos = world_light_tri.points[1] + camera_offset;
			renderable_light_tri.vertices[2].pos = world_light_tri.points[2] + camera_offset;

			renderable_light_tri.vertices[0].color = light.color;
			renderable_light_tri.vertices[1].color = light.color;
			renderable_light_tri.vertices[2].color = light.color;

			output.push_triangle(renderable_light_tri);
		}

		//for (size_t d = 0; d < r.get_num_discontinuities(); ++d) {
		//	const auto world_discontinuity = *r.get_discontinuity(d);
		//	
		//	if (!world_discontinuity.is_boundary) {
		//		vertex_triangle renderable_light_tri;
		//
		//		const float distance_from_light = (requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//		const float angle = 80.f / ((distance_from_light+0.1f)/50.f);
		//		
		//		//(requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//
		//		if (world_discontinuity.winding == world_discontinuity.RIGHT) {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(-angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//		else {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//
		//		renderable_light_tri.vertices[0].color = light.color;
		//		renderable_light_tri.vertices[1].color = light.color;
		//		renderable_light_tri.vertices[2].color = light.color;
		//
		//		output.push_triangle(renderable_light_tri);
		//	}
		//}


		vec2 light_displacement = vec2(cache.all_variation_values[6], cache.all_variation_values[7]);

		auto screen_pos = requests[i].eye_transform - camera_transform;
		screen_pos.pos.x += camera_size.x * 0.5f;
		screen_pos.pos.y = camera_size.y - (screen_pos.pos.y + camera_size.y * 0.5f);
		screen_pos += light_displacement;

		glUniform2f(light_pos_uniform, screen_pos.pos.x, screen_pos.pos.y);

		glUniform1f(light_max_distance_uniform, light.max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			cache.all_variation_values[0] + light.constant.base_value,
			cache.all_variation_values[1] + light.linear.base_value,
			cache.all_variation_values[2] + light.quadratic.base_value
		);

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);

		output.call_triangles();
		output.clear_triangles();
		
		light_program.use();

		glUniform1f(light_max_distance_uniform, light.wall_max_distance.base_value);
		
		glUniform3f(light_attenuation_uniform,
			cache.all_variation_values[3] + light.wall_constant.base_value,
			cache.all_variation_values[4] + light.wall_linear.base_value,
			cache.all_variation_values[5] + light.wall_quadratic.base_value
		);
		
		glUniform3f(light_multiply_color_uniform,
			light.color.r/255.f,
			light.color.g/255.f,
			light.color.b/255.f);
		
		render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::DYNAMIC_BODY], step.camera_state, renderable_drawing_type::NORMAL);

		output.call_triangles();
		output.clear_triangles();

		glUniform3f(light_multiply_color_uniform,
			1.f,
			1.f,
			1.f);
	}

	default_program.use();

	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::DYNAMIC_BODY], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::FLYING_BULLETS], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::CAR_INTERIOR], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::CAR_WHEEL], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::EFFECTS], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::ON_GROUND], step.camera_state, renderable_drawing_type::NEON_MAPS);
	render_system().draw_entities(interp, output.triangles, step.visible_per_layer[render_layer::ON_TILED_FLOOR], step.camera_state, renderable_drawing_type::NEON_MAPS);

	{
		particles_simulation_system::drawing_input in(output.triangles);
		in.camera = step.camera_state.camera;
		in.use_neon_map = true;

		particles.draw(render_layer::EFFECTS, in);
	}

	output.call_triangles();
	output.clear_triangles();

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;

	graphics::fbo::use_default();

	output.set_active_texture(2);
	output.bind_texture(output.light_fbo);
	output.set_active_texture(0);
}