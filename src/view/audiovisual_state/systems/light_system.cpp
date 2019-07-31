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
#include "view/rendering_scripts/draw_character_glow.h"
#include "augs/graphics/shader.hpp"

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
	const auto delta = dt.in_seconds();

	cosm.for_each_having<components::light>(
		[&](const auto it) {
			const auto& light = it.template get<components::light>();
			auto& cache = per_entity_cache[it];

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

void light_system::gather_vis_requests(const light_system_input& in) const {
	auto& requests = thread_local_visibility_requests();

	const auto& cosm = in.cosm;
	const auto& interp  = in.make_drawing_in().interp;

	const auto cone = in.cone;

	const auto queried_camera_aabb = [&]() {
		auto c = cone;
		c.eye.zoom /= in.camera_query_mult;

		return c.get_visible_world_rect_aabb();
	}();

	cosm.template for_each_having<components::light>(
		[&](const auto light_entity) {
			const auto light_transform = light_entity.get_viewing_transform(interp);
			const auto& light = light_entity.template get<components::light>();

			const auto reach = light.calc_reach_trimmed();
			const auto light_aabb = xywh::center_and_size(light_transform.pos, reach);

			if (const auto cache = mapped_or_nullptr(per_entity_cache, unversioned_entity_id(light_entity))) {
				const auto light_displacement = vec2(cache->all_variation_values[6], cache->all_variation_values[7]);

				messages::visibility_information_request request;

				request.eye_transform = light_transform;
				request.eye_transform.pos += light_displacement;

				if (queried_camera_aabb.hover(light_aabb)) {
					request.filter = predefined_queries::line_of_sight();
					request.queried_rect = reach;
					request.subject = light_entity;

					requests.emplace_back(std::move(request));
				}
			}
		}
	);
}

void light_system::render_all_lights(const light_system_input in) const {
	augs::graphics::fbo::mark_current(in.renderer);

	auto& renderer = in.renderer;

	auto set_uniform = [&](auto&& sh, auto&&... args) {
		sh.set_uniform(renderer, std::forward<decltype(args)>(args)...);
	};
	
	auto& performance = in.profiler;
	auto scope = measure_scope(performance.light_rendering);

	const auto& get_drawing_in = in.make_drawing_in;
	const auto global_time_seconds = get_drawing_in().global_time_seconds;
	const auto& light_shader = in.light_shader;
	const auto& wall_light_shader = in.textured_light_shader;

	const auto& standard_shader = in.standard_shader;

	const auto& cosm = in.cosm;

	ensure_eq(0, renderer.get_triangle_count());

	in.light_fbo.set_as_current(renderer);
	
	renderer.set_clear_color(cosm.get_common_significant().ambient_light_color);
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	renderer.set_additive_blending();
	standard_shader.set_as_current(renderer);

	auto make_helper = [&]() {
		return helper_drawer {
			in.visible,
			cosm,
			get_drawing_in(),
			in.total_layer_scope
		};
	};

	make_helper().draw_neons<
		render_layer::FLOOR_AND_ROAD,
		render_layer::ON_FLOOR
	>();

	renderer.call_and_clear_triangles();

	make_helper().draw<
		render_layer::FLOOR_NEON_OVERLAY
	>();

	renderer.set_overwriting_blending();
	renderer.call_and_clear_triangles();
	renderer.set_additive_blending();

	light_shader.set_as_current(renderer);

	using U = augs::common_uniform_name;

	struct light_uniforms {
		U pos = U::light_pos;
		U distance_mult = U::distance_mult;
		U attenuation = U::light_attenuation;
		U multiply_color = U::multiply_color;

		light_uniforms(const augs::graphics::shader_program& s) {
			(void)s;
		}
	};

	const auto light_uniform = light_uniforms(light_shader);
	const auto& wall_light_uniform = light_uniforms(wall_light_shader);

	const auto& interp = get_drawing_in().interp;

	auto& requests = thread_local_visibility_requests();
	auto& responses = thread_local_visibility_responses();

	const auto cone = in.cone;
	const auto eye = cone.eye;

	light_shader.set_projection(renderer, in.cone.get_projection_matrix());
	set_uniform(light_shader, light_uniform.distance_mult, 1.f / eye.zoom);

	const auto queried_camera_aabb = [&]() {
		auto c = cone;
		c.eye.zoom /= in.camera_query_mult;

		return c.get_visible_world_rect_aabb();
	}();

	renderer.set_additive_blending();

	std::size_t num_lights = 0;
	std::size_t num_wall_lights = 0;

	for (size_t i = 0; i < requests.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosm[requests[i].subject];
		const auto maybe_light = light_entity.find<components::light>();

		if (maybe_light == nullptr) {
			continue;
		}

		const auto& light = *maybe_light;
		const auto world_light_pos = requests[i].eye_transform.pos;
		
		const auto& cache = per_entity_cache.at(light_entity);
		const auto& variation_vals = cache.all_variation_values;

		{
			const auto light_frag_pos = [&]() {
				auto screen_space = cone.to_screen_space(world_light_pos);	
				screen_space.y = cone.screen_size.y - screen_space.y;
				return screen_space;
			}();

			set_uniform(light_shader, light_uniform.pos, light_frag_pos);
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
				const auto& a = light.attenuation;

				const auto attenuations = vec3 {
					(variation_vals[0] + a.constant) / CONST_MULT,
					(variation_vals[1] + a.linear) / LINEAR_MULT,
					(variation_vals[2] + a.quadratic) / QUADRATIC_MULT
				};

				set_uniform(light_shader, light_uniform.attenuation, attenuations);
			}

			set_uniform(
				light_shader, 
				light_uniform.multiply_color,
				white.rgb()
			);

			renderer.call_and_clear_triangles();
		}

	}

	if (std::addressof(wall_light_shader) != std::addressof(light_shader)) {
		wall_light_shader.set_as_current(renderer);
		wall_light_shader.set_projection(renderer, in.cone.get_projection_matrix());
		set_uniform(wall_light_shader, wall_light_uniform.distance_mult, 1.f / eye.zoom);
	}

	for (size_t i = 0; i < requests.size(); ++i) {
		const auto& light_entity = cosm[requests[i].subject];
		const auto maybe_light = light_entity.find<components::light>();

		if (maybe_light == nullptr) {
			continue;
		}

		const auto& light = *maybe_light;

		const auto world_light_pos = requests[i].eye_transform.pos;

		const auto& cache = per_entity_cache.at(light_entity);
		const auto& variation_vals = cache.all_variation_values;

		const auto wall_light_aabb = [&]() {
			const auto wall_reach = light.calc_wall_reach_trimmed();
			return xywh::center_and_size(world_light_pos, wall_reach);
		}();

		if (queried_camera_aabb.hover(wall_light_aabb)) {
			const auto light_frag_pos = [&]() {
				auto screen_space = cone.to_screen_space(world_light_pos);	
				screen_space.y = cone.screen_size.y - screen_space.y;
				return screen_space;
			}();

			set_uniform(wall_light_shader, wall_light_uniform.pos, light_frag_pos);

			++num_wall_lights;

			{
				const auto& a = light.wall_attenuation;

				const auto attenuations = vec3 {
					(variation_vals[3] + a.constant) / CONST_MULT,
					(variation_vals[4] + a.linear) / LINEAR_MULT,
					(variation_vals[5] + a.quadratic) / QUADRATIC_MULT
				};

				set_uniform(wall_light_shader, wall_light_uniform.attenuation, attenuations);
			}

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.multiply_color,
				light.color.rgb()
			);

			make_helper().draw<
				render_layer::DYNAMIC_BODY,
				render_layer::OVER_DYNAMIC_BODY,
				render_layer::OVER_SENTIENCES
			>();

			renderer.call_and_clear_triangles();

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.multiply_color,
				white.rgb()
			);
		}
	}

	performance.num_drawn_lights.measure(num_lights);
	performance.num_drawn_wall_lights.measure(num_wall_lights);

	standard_shader.set_as_current(renderer);

	/* Draw neon maps */

	make_helper().draw_neons<
		render_layer::DYNAMIC_BODY,
		render_layer::OVER_DYNAMIC_BODY,
		render_layer::GLASS_BODY,
		render_layer::SMALL_DYNAMIC_BODY,
		render_layer::OVER_SMALL_DYNAMIC_BODY,
		render_layer::OVER_SENTIENCES
	>();

#if BUILD_STENCIL_BUFFER
	{
		auto draw_lights_for = [&](const auto& handle) {
			::draw_neon_map(handle, get_drawing_in());
			::draw_character_glow(
				handle, 
				{
					augs::drawer{ renderer.get_triangle_buffer() },
					interp,
					global_time_seconds,
					in.cast_highlight_tex
				}
			);
		};

		if (const auto fog_of_war_character = cosm[in.fog_of_war_character ? *in.fog_of_war_character : entity_id()]) {
			renderer.call_and_clear_triangles();
			in.fill_stencil();
			renderer.stencil_positive_test();
			standard_shader.set_as_current(renderer);

			in.visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto& handle) {
				if (handle.get_official_faction() != fog_of_war_character.get_official_faction()) {
					draw_lights_for(handle);
				}
			});
			
			renderer.call_and_clear_triangles();

			renderer.set_stencil(false);

			in.visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto& handle) {
				if (handle.get_official_faction() == fog_of_war_character.get_official_faction()) {
					draw_lights_for(handle);
				}
			});
		}
		else {
			in.visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto& handle) {
				draw_lights_for(handle);
			});
		}
	}
#else
	make_helper().draw_neons<render_layer::SENTIENCES>();
#endif

	make_helper().draw_neons<
		render_layer::FLYING_BULLETS,
		render_layer::WATER_COLOR_OVERLAYS,
		render_layer::WATER_SURFACES,
		render_layer::CAR_INTERIOR,
		render_layer::CAR_WHEEL,
		render_layer::NEON_CAPTIONS,
		render_layer::ON_ON_FLOOR,
		render_layer::PLANTED_BOMBS,
		render_layer::AQUARIUM_FLOWERS,
		render_layer::BOTTOM_FISH,
		render_layer::UPPER_FISH,
		render_layer::INSECTS,
		render_layer::AQUARIUM_BUBBLES
	>();


	in.neon_callback();

	renderer.call_and_clear_triangles();

	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_marked(in.renderer);

	renderer.set_active_texture(2);
	in.light_fbo.get_texture().set_as_current(renderer);
	renderer.set_active_texture(0);
}