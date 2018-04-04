#include "augs/misc/randomization.h"

#include "augs/graphics/vertex.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/fbo.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

#include "game/components/light_component.h"
#include "game/components/render_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/visibility_information.h"

#include "game/stateless_systems/visibility_system.h"

#include "view/frame_profiler.h"
#include "view/viewables/game_image.h"
#include "view/rendering_scripts/draw_entity.h"

#include "view/audiovisual_state/systems/light_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"

#define CONST_MULT 100
#define LINEAR_MULT 10000
#define QUADRATIC_MULT 10000000

thread_local randomization rng;

void light_system::reserve_caches_for_entities(const std::size_t n) {
	per_entity_cache.reserve(n);
}

void light_system::clear() {
	per_entity_cache.clear();
}

void light_system::advance_attenuation_variations(
	const cosmos& cosmos,
	const augs::delta dt
) {
	global_time_seconds += dt.in_seconds();

	cosmos.for_each_having<components::light>(
		[&](const auto it) {
			const auto& light = it.template get<invariants::light>();
			auto& cache = per_entity_cache[it];

			const auto delta = dt.in_seconds();

			light.constant.variation.update_value(rng, cache.all_variation_values[0], delta);
			light.linear.variation.update_value(rng, cache.all_variation_values[1], delta);
			light.quadratic.variation.update_value(rng, cache.all_variation_values[2], delta);

			light.wall_constant.variation.update_value(rng, cache.all_variation_values[3], delta);
			light.wall_linear.variation.update_value(rng, cache.all_variation_values[4], delta);
			light.wall_quadratic.variation.update_value(rng, cache.all_variation_values[5], delta);

			light.position_variations[0].update_value(rng, cache.all_variation_values[6], delta);
			light.position_variations[1].update_value(rng, cache.all_variation_values[7], delta);
		}
	);
}

void light_system::render_all_lights(const light_system_input in) const {
	auto& renderer = in.renderer;
	auto& performance = in.profiler;
	
	const auto output = augs::drawer{ renderer.get_triangle_buffer() };
	const auto& light_shader = in.light_shader;
	const auto& standard_shader = in.standard_shader;
	const auto projection_matrix = in.projection_matrix;

	const auto& cosmos = in.cosm;

	ensure_eq(0, renderer.get_triangle_count());

	in.light_fbo.set_as_current();
	
	renderer.set_clear_color(cosmos.get_common_significant().ambient_light_color);
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	light_shader.set_as_current();

	const auto light_pos_uniform = light_shader.get_uniform_location("light_pos");
	const auto light_max_distance_uniform = light_shader.get_uniform_location("max_distance");
	const auto light_distance_mult_uniform = light_shader.get_uniform_location("distance_mult");
	const auto light_attenuation_uniform = light_shader.get_uniform_location("light_attenuation");
	const auto light_multiply_color_uniform = light_shader.get_uniform_location("multiply_color");
	const auto& interp = in.interpolation;
	const auto& particles = in.particles;
	
	const auto& visible_per_layer = in.visible_per_layer;

	std::vector<messages::visibility_information_request> requests;
	std::vector<messages::visibility_information_response> responses;

	light_shader.set_projection(in.camera.get_projection_matrix(in.screen_size));
	light_shader.set_uniform(light_distance_mult_uniform, 1.f / in.camera.zoom);

	const auto camera_aabb = in.camera.get_visible_world_rect_aabb(in.screen_size);

	auto draw_layer = [&](const render_layer r) {
		for (const auto e : visible_per_layer[r]) {
			draw_entity(cosmos[e], { output, in.game_images, global_time_seconds }, in.interpolation);
		}
	};
	
	auto draw_neons = [&](const render_layer r) {
		for (const auto e : visible_per_layer[r]) {
			draw_neon_map(cosmos[e], { output, in.game_images, global_time_seconds }, in.interpolation);
		}
	};

	{
		auto scope = measure_scope(performance.light_visibility);

		cosmos.for_each_having<components::light>(
			[&](const auto light_entity) {
				if (!camera_aabb.hover(*light_entity.find_aabb())) {
					return;
				}

				if (const auto cache = mapped_or_nullptr(per_entity_cache, light_entity.get_id())) {
					const auto light_displacement = vec2(cache->all_variation_values[6], cache->all_variation_values[7]);

					messages::visibility_information_request request;
					request.eye_transform = light_entity.get_viewing_transform(interp);
					request.eye_transform.pos += light_displacement;
					request.filter = filters::line_of_sight_query();
					request.square_side = light_entity.template get<invariants::light>().max_distance.base_value;
					request.subject = light_entity;

					requests.push_back(request);
				}
			}
		);

		std::vector<messages::line_of_sight_response> dummy;
		visibility_system(DEBUG_FRAME_LINES).respond_to_visibility_information_requests(cosmos, {}, requests, dummy, responses);

		performance.num_visible_lights.measure(requests.size());
	}

	auto scope = measure_scope(performance.light_rendering);

	renderer.set_additive_blending();

	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosmos[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		const auto& light_def = light_entity.get<invariants::light>();
		const auto world_light_pos = requests[i].eye_transform.pos;

		for (size_t t = 0; t < r.get_num_triangles(); ++t) {
			const auto world_light_tri = r.get_world_triangle(t, world_light_pos);
			augs::vertex_triangle renderable_light_tri;

			renderable_light_tri.vertices[0].pos = world_light_tri[0];
			renderable_light_tri.vertices[1].pos = world_light_tri[1];
			renderable_light_tri.vertices[2].pos = world_light_tri[2];

			auto considered_color = light.color;
			
			if (considered_color == black) {
				considered_color.set_hsv({ fmod(global_time_seconds / 16.f, 1.f), 1.0, 1.0 });
			}

			renderable_light_tri.vertices[0].color = considered_color;
			renderable_light_tri.vertices[1].color = considered_color;
			renderable_light_tri.vertices[2].color = considered_color;

			renderer.push_triangle(renderable_light_tri);
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
		//		renderer.push_triangle(renderable_light_tri);
		//	}
		//}

		const auto& cache = per_entity_cache.at(light_entity);

		const auto light_frag_pos = in.camera.to_screen_space(in.screen_size, world_light_pos);

		light_shader.set_uniform(light_pos_uniform, light_frag_pos);
		light_shader.set_uniform(light_max_distance_uniform, light_def.max_distance.base_value);
		
		light_shader.set_uniform(
			light_attenuation_uniform,
			vec3 {
				(cache.all_variation_values[0] + light_def.constant.base_value) / CONST_MULT,
				(cache.all_variation_values[1] + light_def.linear.base_value) / LINEAR_MULT,
				(cache.all_variation_values[2] + light_def.quadratic.base_value) / QUADRATIC_MULT
			}
		);
		
		light_shader.set_uniform(
			light_multiply_color_uniform,
			white.rgb()
		);

		renderer.call_triangles();
		renderer.clear_triangles();
		
		light_shader.set_as_current();

		light_shader.set_uniform(light_max_distance_uniform, light_def.wall_max_distance.base_value);
		
		light_shader.set_uniform(light_attenuation_uniform,
			vec3 {
				(cache.all_variation_values[3] + light_def.wall_constant.base_value) / CONST_MULT,
				(cache.all_variation_values[4] + light_def.wall_linear.base_value) / LINEAR_MULT,
				(cache.all_variation_values[5] + light_def.wall_quadratic.base_value) / QUADRATIC_MULT
			}
		);
		
		light_shader.set_uniform(
			light_multiply_color_uniform,
			light.color.rgb()
		);
		
		draw_layer(render_layer::DYNAMIC_BODY);

		renderer.call_triangles();
		renderer.clear_triangles();

		light_shader.set_uniform(
			light_multiply_color_uniform,
			white.rgb()
		);
	}

	standard_shader.set_as_current();

	draw_neons(render_layer::DYNAMIC_BODY);
	draw_neons(render_layer::SMALL_DYNAMIC_BODY);
	draw_neons(render_layer::FLYING_BULLETS);
	draw_neons(render_layer::CAR_INTERIOR);
	draw_neons(render_layer::CAR_WHEEL);
	draw_neons(render_layer::NEON_CAPTIONS);
	draw_neons(render_layer::ON_GROUND);

	{
		invariants::sprite::drawing_input basic(output);
		basic.use_neon_map = true;

		particles.draw_particles_as_sprites(
			in.game_images,
			basic,
			render_layer::ILLUMINATING_PARTICLES
		);
	}

	in.neon_callback();

	renderer.call_triangles();
	renderer.clear_triangles();

	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_none();

	renderer.set_active_texture(2);
	in.light_fbo.get_texture().bind();
	renderer.set_active_texture(0);
}