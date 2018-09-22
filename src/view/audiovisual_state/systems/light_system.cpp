#include "augs/graphics/vertex.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/fbo.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"

#include "game/enums/filters.h"

#include "game/components/light_component.h"
#include "game/components/render_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/visibility_information.h"

#include "game/stateless_systems/visibility_system.h"

#include "view/frame_profiler.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/rendering_scripts/draw_entity.h"
#include "view/rendering_scripts/helper_drawer.h"

#include "view/audiovisual_state/systems/light_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"

#define CONST_MULT 100
#define LINEAR_MULT 10000
#define QUADRATIC_MULT 10000000

void light_system::reserve_caches_for_entities(const std::size_t n) {
	per_entity_cache.reserve(n);
}

void light_system::clear() {
	per_entity_cache.clear();
}

void light_system::advance_attenuation_variations(
	randomization& rng,
	const cosmos& cosm,
	const augs::delta dt
) {
	cosm.for_each_having<components::light>(
		[&](const auto it) {
			const auto& light = it.template get<invariants::light>();
			auto& cache = per_entity_cache[it];

			const auto delta = dt.in_seconds();

			auto& vals = cache.all_variation_values;

			if (light.variation.is_enabled) {
				auto& v = light.variation.value;

				v.constant.update_value(rng, vals[0], delta);
				v.linear.update_value(rng, vals[1], delta);
				v.quadratic.update_value(rng, vals[2], delta);
			}
			else {
				vals[0] = vals[1] = vals[2] = 0.f;
			}

			if (light.wall_variation.is_enabled) {
				auto& v = light.wall_variation.value;

				v.constant.update_value(rng, vals[3], delta);
				v.linear.update_value(rng, vals[4], delta);
				v.quadratic.update_value(rng, vals[5], delta);
			}
			else {
				vals[3] = vals[4] = vals[5] = 0.f;
			}

			if (light.position_variations.is_enabled) {
				auto& v = light.position_variations.value;

				v[0].update_value(rng, vals[6], delta);
				v[1].update_value(rng, vals[7], delta);
			}
			else {
				vals[6] = vals[7] = 0.f;
			}
		}
	);
}

void light_system::render_all_lights(const light_system_input in) const {
	auto& renderer = in.renderer;
	auto& performance = in.profiler;

	const auto drawing_in = in.drawing_in;
	const auto global_time_seconds = drawing_in.global_time_seconds;
	const auto output = augs::drawer{ renderer.get_triangle_buffer() };
	const auto& light_shader = in.light_shader;
	const auto& wall_light_shader = light_shader;

	const auto& standard_shader = in.standard_shader;

	const auto& cosm = in.cosm;

	auto light_raycasts_scope = cosm.measure_raycasts(performance.light_raycasts);

	ensure_eq(0, renderer.get_triangle_count());

	in.light_fbo.set_as_current();
	
	renderer.set_clear_color(cosm.get_common_significant().ambient_light_color);
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	light_shader.set_as_current();

	struct light_uniforms {
		GLint pos;
		GLint distance_mult;
		GLint attenuation;
		GLint multiply_color;

		light_uniforms(const augs::graphics::shader_program& s) {
			pos = s.get_uniform_location("light_pos");
			distance_mult = s.get_uniform_location("distance_mult");
			attenuation = s.get_uniform_location("light_attenuation");
			multiply_color = s.get_uniform_location("multiply_color");
		}
	};

	const auto light_uniform = light_uniforms(light_shader);
	const auto& wall_light_uniform = light_uniform;

	const auto& interp = drawing_in.interp;
	const auto& particles = in.particles;
	
	const auto helper = helper_drawer {
		in.visible,
		cosm,
		drawing_in,
		in.total_layer_scope
	};

	visibility_requests requests;
	visibility_responses responses;

	const auto cone = in.cone;
	const auto eye = cone.eye;

	light_shader.set_projection(in.cone.get_projection_matrix());
	light_shader.set_uniform(light_uniform.distance_mult, 1.f / eye.zoom);

	const auto queried_camera_aabb = [&]() {
		auto c = cone;
		c.eye.zoom /= in.camera_query_mult;

		return c.get_visible_world_rect_aabb();
	}();

	{
		auto scope = measure_scope(performance.light_visibility);

		cosm.for_each_having<components::light>(
			[&](const auto light_entity) {
				const auto light_transform = light_entity.get_viewing_transform(interp);
				const auto& light_def = light_entity.template get<invariants::light>();

				const auto reach = light_def.calc_reach_trimmed();
				const auto light_aabb = xywh::center_and_size(light_transform.pos, vec2::square(reach * 2));

				if (const auto cache = mapped_or_nullptr(per_entity_cache, unversioned_entity_id(light_entity))) {
					const auto light_displacement = vec2(cache->all_variation_values[6], cache->all_variation_values[7]);

					messages::visibility_information_request request;

					request.eye_transform = light_transform;
					request.eye_transform.pos += light_displacement;

					if (queried_camera_aabb.hover(light_aabb)) {
						request.filter = filters::line_of_sight_query();
						request.square_side = reach * 2;
					}
					else {
						request.square_side = -1.f;
					}

					request.subject = light_entity;

					requests.emplace_back(std::move(request));
				}
			}
		);

		visibility_system(DEBUG_FRAME_LINES).calc_visibility(cosm, requests, responses);
	}

	auto scope = measure_scope(performance.light_rendering);

	renderer.set_additive_blending();

	std::size_t num_lights = 0;
	std::size_t num_wall_lights = 0;

	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosm[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		const auto& light_def = light_entity.get<invariants::light>();
		const auto world_light_pos = requests[i].eye_transform.pos;
		
		const auto& cache = per_entity_cache.at(light_entity);
		const auto& variation_vals = cache.all_variation_values;

		{
			const auto light_frag_pos = [&]() {
				auto screen_space = cone.to_screen_space(world_light_pos);	
				screen_space.y = cone.screen_size.y - screen_space.y;
				return screen_space;
			}();

			light_shader.set_uniform(light_uniform.pos, light_frag_pos);
		}

		const bool was_visibility_calculated = !r.empty();

		if (was_visibility_calculated) {
			++num_lights;

			for (std::size_t t = 0; t < r.get_num_triangles(); ++t) {
				const auto world_light_tri = r.get_world_triangle(t, world_light_pos);
				augs::vertex_triangle renderable_light_tri;

				renderable_light_tri.vertices[0].pos = world_light_tri[0];
				renderable_light_tri.vertices[1].pos = world_light_tri[1];
				renderable_light_tri.vertices[2].pos = world_light_tri[2];

				auto considered_color = light.color;

				if (considered_color == black) {
					/* Easter egg: a completely black light gives a color wave. Pretty suitable for parties. */
					considered_color.set_hsv({ fmod(global_time_seconds / 16.f, 1.f), 1.0, 1.0 });
				}

				renderable_light_tri.vertices[0].color = considered_color;
				renderable_light_tri.vertices[1].color = considered_color;
				renderable_light_tri.vertices[2].color = considered_color;

				renderer.push_triangle(renderable_light_tri);
			}

			{
				const auto& a = light_def.attenuation;

				const auto attenuations = vec3 {
					(variation_vals[0] + a.constant) / CONST_MULT,
					(variation_vals[1] + a.linear) / LINEAR_MULT,
					(variation_vals[2] + a.quadratic) / QUADRATIC_MULT
				};

				light_shader.set_uniform(light_uniform.attenuation, attenuations);
			}

			light_shader.set_uniform(
				light_uniform.multiply_color,
				white.rgb()
			);

			renderer.call_triangles();
			renderer.clear_triangles();
		}

	}

	if (std::addressof(wall_light_shader) != std::addressof(light_shader)) {
		wall_light_shader.set_as_current();
		wall_light_shader.set_projection(in.cone.get_projection_matrix());
		wall_light_shader.set_uniform(wall_light_uniform.distance_mult, 1.f / eye.zoom);
	}

	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& light_entity = cosm[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		const auto& light_def = light_entity.get<invariants::light>();
		const auto world_light_pos = requests[i].eye_transform.pos;

		const auto& cache = per_entity_cache.at(light_entity);
		const auto& variation_vals = cache.all_variation_values;

		const auto wall_light_aabb = [&]() {
			const auto wall_reach = light_def.calc_wall_reach_trimmed();
			return xywh::center_and_size(world_light_pos, vec2::square(wall_reach * 2));
		}();

		if (queried_camera_aabb.hover(wall_light_aabb)) {
			const auto light_frag_pos = [&]() {
				auto screen_space = cone.to_screen_space(world_light_pos);	
				screen_space.y = cone.screen_size.y - screen_space.y;
				return screen_space;
			}();

			wall_light_shader.set_uniform(wall_light_uniform.pos, light_frag_pos);

			++num_wall_lights;

			{
				const auto& a = light_def.wall_attenuation;

				const auto attenuations = vec3 {
					(variation_vals[3] + a.constant) / CONST_MULT,
					(variation_vals[4] + a.linear) / LINEAR_MULT,
					(variation_vals[5] + a.quadratic) / QUADRATIC_MULT
				};

				wall_light_shader.set_uniform(wall_light_uniform.attenuation, attenuations);
			}

			wall_light_shader.set_uniform(
				wall_light_uniform.multiply_color,
				light.color.rgb()
			);

			helper.draw<
				render_layer::DYNAMIC_BODY,
				render_layer::OVER_DYNAMIC_BODY
			>();

			renderer.call_triangles();
			renderer.clear_triangles();

			wall_light_shader.set_uniform(
				wall_light_uniform.multiply_color,
				white.rgb()
			);
		}
	}

	performance.num_drawn_lights.measure(num_lights);
	performance.num_drawn_wall_lights.measure(num_wall_lights);

	standard_shader.set_as_current();

	/* Draw neon maps */

	helper.draw_neons<
		render_layer::DYNAMIC_BODY,
		render_layer::OVER_DYNAMIC_BODY,
		render_layer::GLASS_BODY,
		render_layer::SMALL_DYNAMIC_BODY,
		render_layer::OVER_SMALL_DYNAMIC_BODY
	>();

#if BUILD_STENCIL_BUFFER
	if (const auto fog_of_war_character = cosm[in.fog_of_war_character ? *in.fog_of_war_character : entity_id()]) {
		renderer.call_and_clear_triangles();
		in.fill_stencil();
		renderer.stencil_positive_test();
		standard_shader.set_as_current();

		helper.visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
			if (handle.get_official_faction() != fog_of_war_character.get_official_faction()) {
				::draw_neon_map(handle, drawing_in);
			}
		});

		renderer.call_and_clear_triangles();

		renderer.disable_stencil();

		helper.visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
			if (handle.get_official_faction() == fog_of_war_character.get_official_faction()) {
				::draw_neon_map(handle, drawing_in);
			}
		});
	}
	else {
		helper.draw_neons<render_layer::SENTIENCES>();
	}
#else
	helper.draw_neons<render_layer::SENTIENCES>();
#endif

	helper.draw_neons<
		render_layer::FLYING_BULLETS,
		render_layer::WATER_COLOR_OVERLAYS,
		render_layer::WATER_SURFACES,
		render_layer::CAR_INTERIOR,
		render_layer::CAR_WHEEL,
		render_layer::NEON_CAPTIONS,
		render_layer::FLOOR_AND_ROAD,
		render_layer::ON_FLOOR,
		render_layer::ON_ON_FLOOR,
		render_layer::PLANTED_BOMBS,
		render_layer::AQUARIUM_FLOWERS,
		render_layer::BOTTOM_FISH,
		render_layer::UPPER_FISH,
		render_layer::AQUARIUM_BUBBLES
	>();

	particles.draw_particles(
		drawing_in.manager,
		in.plain_animations,
		draw_particles_input { output, true },
		particle_layer::ILLUMINATING_PARTICLES
	);

	in.neon_callback();

	renderer.call_and_clear_triangles();

	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_none();

	renderer.set_active_texture(2);
	in.light_fbo.get_texture().bind();
	renderer.set_active_texture(0);
}