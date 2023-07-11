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

#include "view/audiovisual_state/systems/light_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "augs/graphics/shader.hpp"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

void light_system::reserve_caches_for_entities(const std::size_t) {

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

struct light_uniforms {
	using U = augs::common_uniform_name;

	U pos = U::light_pos;
	U distance_mult = U::distance_mult;
	U max_distance = U::max_distance;
	U cutoff_distance = U::cutoff_distance;
	U attenuation = U::light_attenuation;
	U multiply_color = U::multiply_color;

	light_uniforms(const augs::graphics::shader_program& s) {
		(void)s;
	}
};

void light_system::render_all_lights(const light_system_input in) const {
	using D = augs::dedicated_buffer;

	const auto& light_shader = in.light_shader;
	const auto& wall_light_shader = in.textured_light_shader;

	const auto light_uniform = light_uniforms(light_shader);
	const auto& wall_light_uniform = light_uniforms(wall_light_shader);

	const auto& standard_shader = in.standard_shader;
	const auto& cosm = in.cosm;

	const auto& light_requests = in.requests;

	const auto queried_camera_aabb = in.queried_cone.get_visible_world_rect_aabb(); 
	const auto cone = in.cone;
	const auto eye = cone.eye;

	auto& performance = in.profiler;
	auto scope = measure_scope(performance.light_rendering);

	auto& renderer = in.renderer;
	ensure_eq(static_cast<std::size_t>(0), renderer.get_triangle_count());

	auto num_lights = std::size_t(0);
	auto num_wall_lights = std::size_t(0);

	auto set_uniform = [&](auto&& sh, auto&&... args) {
		sh.set_uniform(renderer, std::forward<decltype(args)>(args)...);
	};

	auto setup_light_shader = [&]() {
		light_shader.set_as_current(renderer);
		light_shader.set_projection(renderer, in.cone.get_projection_matrix());

		set_uniform(light_shader, light_uniform.distance_mult, 1.f / eye.zoom);

		renderer.set_additive_blending();
	};

	auto overlay_light_polygons = [&]() {
		for (std::size_t i = 0; i < light_requests.size(); ++i) {
			const auto& request = light_requests[i];
			const auto& light_entity = cosm[request.subject];
			const auto maybe_light = light_entity.find<components::light>();

			if (maybe_light == nullptr) {
				continue;
			}

			const auto& light = *maybe_light;
			const auto world_light_pos = request.eye_transform.pos;
			
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

			const bool was_visibility_calculated = request.valid();

			if (was_visibility_calculated) {
				++num_lights;

				{
					const auto& a = light.attenuation;

					const auto attenuations = vec3 {
						(std::abs(variation_vals[0]) + a.constant) / CONST_MULT,
						(std::abs(variation_vals[1]) + a.linear) / LINEAR_MULT,
						(std::abs(variation_vals[2]) + a.quadratic) / QUADRATIC_MULT
					};

					set_uniform(light_shader, light_uniform.attenuation, attenuations);
				}

				set_uniform(
					light_shader, 
					light_uniform.multiply_color,
					white.rgb()
				);

				const auto max_distance = light.attenuation.calc_reach();
				const auto cutoff_distance = std::max(0.0f, max_distance - 3*float(light.attenuation.trim_alpha));

				set_uniform(
					light_shader, 
					light_uniform.max_distance,
					max_distance
				);

				set_uniform(
					light_shader, 
					light_uniform.cutoff_distance,
					cutoff_distance
				);

				renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_VISIBILITY, i);
			}
		}
	};

	auto setup_wall_light_shader = [&]() {
		if (std::addressof(wall_light_shader) != std::addressof(light_shader)) {
			wall_light_shader.set_as_current(renderer);
			wall_light_shader.set_projection(renderer, in.cone.get_projection_matrix());
			set_uniform(wall_light_shader, wall_light_uniform.distance_mult, 1.f / eye.zoom);
		}
	};

	auto overlay_wall_lights = [&]() {
		for (size_t i = 0; i < light_requests.size(); ++i) {
			const auto& request = light_requests[i];
			const auto& light_entity = cosm[request.subject];
			const auto maybe_light = light_entity.find<components::light>();

			if (maybe_light == nullptr) {
				continue;
			}

			const auto& light = *maybe_light;

			const auto world_light_pos = request.eye_transform.pos;

			const auto& cache = per_entity_cache.at(light_entity);
			const auto& variation_vals = cache.all_variation_values;

			const auto wall_light_aabb = [&]() {
				const bool exact = in.perf_settings.wall_light_drawing_precision == accuracy_type::EXACT;
				const auto wall_reach = exact ? light.calc_wall_reach_trimmed() : light.calc_reach_trimmed();

				return xywh::center_and_size(world_light_pos, wall_reach);
			}();

			if (!queried_camera_aabb.hover(wall_light_aabb)) {
				continue;
			}

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
					(std::abs(variation_vals[3]) + a.constant) / CONST_MULT,
					(std::abs(variation_vals[4]) + a.linear) / LINEAR_MULT,
					(std::abs(variation_vals[5]) + a.quadratic) / QUADRATIC_MULT
				};

				set_uniform(wall_light_shader, wall_light_uniform.attenuation, attenuations);
			}

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.multiply_color,
				light.color.rgb()
			);

			const auto max_distance = light.wall_attenuation.calc_reach();
			const auto cutoff_distance = std::max(0.0f, max_distance - 3*float(light.wall_attenuation.trim_alpha));

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.max_distance,
				max_distance
			);

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.cutoff_distance,
				cutoff_distance
			);

			renderer.call_triangles(D::WALL_ILLUMINATIONS);
		}
	};

	auto draw_sentience_neons = [&]() {
		if (const auto fog_of_war_character = cosm[in.fog_of_war_character ? *in.fog_of_war_character : entity_id()]) {
			renderer.call_and_clear_triangles();
			renderer.stencil_positive_test();
			standard_shader.set_as_current(renderer);

			renderer.set_stencil(true);
			renderer.call_triangles(D::NEONS_ENEMY_SENTIENCES);
			renderer.set_stencil(false);

			renderer.call_triangles(D::NEONS_FRIENDLY_SENTIENCES);
		}
		else {
			renderer.call_triangles(D::NEONS_FRIENDLY_SENTIENCES);
		}
	};

	auto restore_renderer = [&]() {
		renderer.set_standard_blending();

		augs::graphics::fbo::set_current_to_marked(in.renderer);

		renderer.set_active_texture(2);
		in.light_fbo.get_texture().set_as_current(renderer);
		renderer.set_active_texture(0);
	};

	/* Flow */

	augs::graphics::fbo::mark_current(in.renderer);

	in.light_fbo.set_as_current(renderer);
	in.write_fow_to_stencil();

	renderer.set_clear_color(cosm.get_common_significant().light.ambient_color);
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	renderer.set_additive_blending();
	standard_shader.set_as_current(renderer);

	renderer.call_triangles(D::GROUND_NEONS);
	renderer.set_standard_blending();
	in.neon_occlusion_callback(false);
	renderer.call_triangles(D::GROUND_NEON_OCCLUDERS);
	renderer.set_additive_blending();

	draw_sentience_neons();

	renderer.call_triangles(D::DROPPED_ITEMS_NEONS);
	renderer.call_triangles(D::UNDER_FOREGROUND_NEONS);

	in.neon_callback();
	renderer.call_and_clear_triangles();

	renderer.set_standard_blending();
	in.neon_occlusion_callback(true);
	renderer.call_triangles(D::FOREGROUND_NEON_OCCLUDERS);
	renderer.set_additive_blending();

	setup_light_shader();
	overlay_light_polygons();
	setup_wall_light_shader();
	overlay_wall_lights();

	standard_shader.set_as_current(renderer);
	renderer.call_triangles(D::FOREGROUND_NEONS);

	renderer.call_and_clear_triangles();

	performance.num_drawn_lights.measure(num_lights);
	performance.num_drawn_wall_lights.measure(num_wall_lights);

	restore_renderer();
}