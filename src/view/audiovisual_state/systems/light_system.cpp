#include <cstddef>
#include "augs/graphics/vertex.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/fbo.h"
#include "augs/graphics/texture.h"

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

/*
	Draws the part of the reach rectangle that a light can illuminate - a polygon circumscribing its circle, clipped to the rectangle.
	Fragments beyond the circle would be discarded by the light shader anyway, so this only saves the work on them.
*/

static void draw_light_area(
	augs::renderer& renderer,
	const ltrb reach,
	const vec2 center,
	const real32 radius,
	const rgba col
) {
	constexpr int num_sides = 32;

	thread_local std::vector<vec2> polygon;
	thread_local std::vector<vec2> clipped;

	polygon.clear();

	const auto circumscribed_radius = radius / std::cos(PI<real32> / num_sides) + 2.0f;

	for (int i = 0; i < num_sides; ++i) {
		polygon.push_back(center + vec2::from_degrees(360.0f * i / num_sides) * circumscribed_radius);
	}

	auto clip = [&](auto inside, auto intersect) {
		clipped.clear();

		for (std::size_t i = 0; i < polygon.size(); ++i) {
			const auto a = polygon[i];
			const auto b = polygon[(i + 1) % polygon.size()];

			const bool a_in = inside(a);
			const bool b_in = inside(b);

			if (a_in) {
				clipped.push_back(a);
			}

			if (a_in != b_in) {
				clipped.push_back(intersect(a, b));
			}
		}

		polygon.swap(clipped);
	};

	auto at_x = [](const vec2 a, const vec2 b, const real32 x) {
		return vec2(x, a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x));
	};

	auto at_y = [](const vec2 a, const vec2 b, const real32 y) {
		return vec2(a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y), y);
	};

	clip([&](const vec2 p) { return p.x >= reach.l; }, [&](const vec2 a, const vec2 b) { return at_x(a, b, reach.l); });
	clip([&](const vec2 p) { return p.x <= reach.r; }, [&](const vec2 a, const vec2 b) { return at_x(a, b, reach.r); });
	clip([&](const vec2 p) { return p.y >= reach.t; }, [&](const vec2 a, const vec2 b) { return at_y(a, b, reach.t); });
	clip([&](const vec2 p) { return p.y <= reach.b; }, [&](const vec2 a, const vec2 b) { return at_y(a, b, reach.b); });

	if (polygon.size() < 3) {
		return;
	}

	for (std::size_t i = 2; i < polygon.size(); ++i) {
		augs::vertex_triangle tri;

		tri.vertices[0].pos = polygon[0];
		tri.vertices[1].pos = polygon[i - 1];
		tri.vertices[2].pos = polygon[i];

		for (auto& v : tri.vertices) {
			v.color = col;
			v.texcoord = vec2::zero;
		}

		renderer.push_triangle(tri);
	}

	renderer.call_and_clear_triangles();
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
		set_uniform(light_shader, augs::common_uniform_name::light_pass, 0);

		renderer.set_additive_blending();
	};

	/*
		The removed light texture holds the light that all shadows removed - for the quantization,
		and the hue light texture - the light that shadows of low obstacles removed, for keeping the hue.
	*/

	const bool keep_hue_in_shadows = 
		in.hue_light_fbo != nullptr
		&& cosm.get_common_significant().light.point_light_hue_preservation > 0.0f
	;

	const bool track_removed_intensity = 
		in.removed_light_fbo != nullptr
		&& in.perf_settings.quantize_lights
	;

	const bool track_removed_light = keep_hue_in_shadows || track_removed_intensity;

	auto set_pass = [&](const int pass) {
		set_uniform(light_shader, augs::common_uniform_name::light_pass, pass);
	};

	auto overlay_light_polygons = [&]() {
		if (track_removed_light) {
			renderer.set_active_texture(6);
			in.light_fbo.get_texture().set_as_current(renderer);
			renderer.set_active_texture(0);
		}

		for (std::size_t i = 0; i < light_requests.size(); ++i) {
			const auto& request = light_requests[i];

			/*
				Use light data from request instead of entity lookup.
			*/

			const auto& light = request.light_data;
			const auto world_light_pos = request.eye_transform.pos;

			/*
				Use variation cache if present, otherwise use zero values.
			*/

			const auto variation_vals = request.variation_cache.value_or(std::array<float, 10>{});

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

				/*
					Every light is its whole reach, scaled by the mask of its shadows
					built in the alpha of the light texture - see light.fsh for the passes.
					Lights with a height build it from their own shadows (light_height_shadows.h),
					the other ones from their visibility polygons.
				*/

				const auto reach = xywh::center_and_size(world_light_pos, request.queried_rect);

				if (light.height > 0.0f) {
					set_pass(3);
					renderer.set_max_blending();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, white);

					set_pass(2);
					renderer.set_min_blending();
					renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_SHADOW_MASKS, i);

					if (keep_hue_in_shadows && !light.hue_through_walls) {
						/*
							The mask holds only the obstacles reaching the ceiling now,
							so the light removed by the lower ones is taken only where walls let it through.
						*/

						in.hue_light_fbo->set_as_current(renderer);

						set_pass(9);
						renderer.set_color_only_additive_blending();
						renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_LOW_SHADOW_MASKS, i);

						in.light_fbo.set_as_current(renderer);

						set_pass(2);
						renderer.set_min_blending();
					}

					renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_LOW_SHADOW_MASKS, i);
				}
				else {
					set_pass(6);
					renderer.set_min_blending();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, white);

					set_pass(8);
					renderer.set_max_blending();
					renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_VISIBILITY, i);
					renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_PENUMBRAS, i);
				}

				set_pass(4);
				renderer.set_dst_alpha_additive_blending();
				::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);

				if (keep_hue_in_shadows && light.hue_through_walls) {
					/*
						The light keeps its hue through all its shadows, walls included.
					*/

					in.hue_light_fbo->set_as_current(renderer);

					set_pass(7);
					renderer.set_color_only_additive_blending();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);

					in.light_fbo.set_as_current(renderer);
				}

				if (track_removed_intensity) {
					/*
						The light all the shadows removed, so that quantized lights brighten shadowed pixels
						by the light they would get without them, keeping penumbras smooth.
					*/

					in.removed_light_fbo->set_as_current(renderer);

					set_pass(7);
					renderer.set_color_only_additive_blending();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);

					in.light_fbo.set_as_current(renderer);
				}
			}
		}

		set_pass(0);
		renderer.set_additive_blending();
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

			/*
				Use light data from request instead of entity lookup.
			*/

			const auto& light = request.light_data;
			const auto world_light_pos = request.eye_transform.pos;

			/*
				Use variation cache if present, otherwise use zero values.
			*/

			const auto variation_vals = request.variation_cache.value_or(std::array<float, 10>{});

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
			renderer.stencil_positive_test();

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

		if (in.removed_light_fbo != nullptr) {
			renderer.set_active_texture(5);
			in.removed_light_fbo->get_texture().set_as_current(renderer);
		}

		if (in.hue_light_fbo != nullptr) {
			renderer.set_active_texture(7);
			in.hue_light_fbo->get_texture().set_as_current(renderer);
		}

		renderer.set_active_texture(0);
	};

	/* Flow */

	augs::graphics::fbo::mark_current(in.renderer);

	/*
		Textures that no pass writes to this frame aren't cleared - nothing reads them either.
	*/

	if (track_removed_intensity) {
		in.removed_light_fbo->set_as_current(renderer);
		renderer.clear_current_fbo();
	}

	if (keep_hue_in_shadows) {
		in.hue_light_fbo->set_as_current(renderer);
		renderer.clear_current_fbo();
	}

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
	renderer.call_and_clear_triangles();
	renderer.call_triangles(D::GROUND_NEON_OCCLUDERS);
	renderer.set_additive_blending();
	standard_shader.set_as_current(renderer);
	draw_sentience_neons();

	if (in.strict_fow) {
		renderer.stencil_positive_test();
		renderer.set_stencil(true);
	}

	renderer.call_triangles(D::DROPPED_ITEMS_NEONS);

	if (in.strict_fow) {
		renderer.set_stencil(false);
	}

	renderer.call_triangles(D::UNDER_FOREGROUND_NEONS);

	if (in.strict_fow) {
		renderer.set_stencil(true);
		renderer.stencil_positive_test();
	}

	/*
		The neon maps of bullets are stretched backward several times to overlap their particle trails,
		which would magnify the atlas' nearest-neighbor filtering into visible blocks.
		Filter this single layer linearly.
	*/

	if (in.general_atlas != nullptr) {
		in.general_atlas->set_filtering(renderer, augs::filtering_type::LINEAR);
	}

	renderer.call_triangles(D::MISSILES_NEONS);

	if (in.general_atlas != nullptr) {
		in.general_atlas->set_filtering(renderer, in.default_filtering);
	}

	if (in.strict_fow) {
		renderer.set_stencil(false);
	}

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
	renderer.call_triangles(D::LYING_CORPSES_NEONS);
	renderer.call_triangles(D::GROUND_DECALS_NEONS);

	renderer.call_and_clear_triangles();

	performance.num_drawn_lights.measure(num_lights);
	performance.num_drawn_wall_lights.measure(num_wall_lights);

	restore_renderer();
}