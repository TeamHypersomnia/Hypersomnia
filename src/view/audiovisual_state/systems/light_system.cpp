#include <cstddef>
#include <algorithm>
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
				light.variation.value.update_value(rng, vals[0], delta);
			}
			else {
				vals[0] = 0.f;
			}

			if (light.wall_variation.is_enabled) {
				light.wall_variation.value.update_value(rng, vals[3], delta);
			}
			else {
				vals[3] = 0.f;
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
	U core = U::light_core;
	U intensity = U::light_intensity;
	U physical = U::physical_lights;
	U multiply_color = U::multiply_color;

	light_uniforms(const augs::graphics::shader_program& s) {
		(void)s;
	}
};

void light_system::render_all_lights(const light_system_input in) const {
	using D = augs::dedicated_buffer;

	const bool physical_lights = in.cosm.get_common_significant().light.physical_lights;

	/*
		variation_val is the flicker of the intensity, see light_value_variation.
		Returns the radius, where the light ends.
	*/

	auto set_falloff_uniforms = [&](
		const auto& shader,
		const light_uniforms& uniforms,
		const light_falloff& falloff,
		const legacy_attenuation& attenuation,
		const float variation_val
	) {
		const auto flicker = 1.0f - std::abs(variation_val);

		if (physical_lights) {
			shader.set_uniform(in.renderer, uniforms.max_distance, falloff.radius);
			shader.set_uniform(in.renderer, uniforms.core, falloff.calc_core());
			shader.set_uniform(in.renderer, uniforms.intensity, falloff.intensity * flicker);

			return falloff.radius;
		}

		const auto max_distance = attenuation.calc_reach();
		const auto cutoff_distance = std::max(0.0f, max_distance - 3 * float(attenuation.trim_alpha));

		/*
			Flickering raises every attenuation term by the same factor.
		*/

		const auto attenuations = vec3 {
			attenuation.constant / CONST_MULT / flicker,
			attenuation.linear / LINEAR_MULT / flicker,
			attenuation.quadratic / QUADRATIC_MULT / flicker
		};

		shader.set_uniform(in.renderer, uniforms.attenuation, attenuations);
		shader.set_uniform(in.renderer, uniforms.max_distance, max_distance);
		shader.set_uniform(in.renderer, uniforms.cutoff_distance, cutoff_distance);

		return max_distance;
	};

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
		set_uniform(light_shader, light_uniform.physical, physical_lights ? 1 : 0);
		set_uniform(light_shader, augs::common_uniform_name::light_pass, 0);

		/*
			For quantized muzzle flashes - the same bands as quantized neons, see posterized_neon.fsh.
		*/

		const auto& light_settings = cosm.get_common_significant().light;

		set_uniform(light_shader, augs::common_uniform_name::ambient_color, light_settings.ambient_color);
		set_uniform(light_shader, augs::common_uniform_name::smooth_brightening, physical_lights && light_settings.brighten_lights ? 1 : 0);

		renderer.set_additive_blending();
	};

	/*
		The hue light texture holds the light that shadows of low obstacles removed, for keeping the hue,
		and the removed light texture - the light that all shadows removed: for brightening legacy lights in bands
		(keeping penumbras smooth, see illuminated.fsh), and it can serve as the hue light (below).
	*/

	const bool keep_hue_in_shadows = 
		in.hue_light_fbo != nullptr
		&& cosm.get_common_significant().light.point_light_shadows.hue_preservation > 0.0f
	;

	/*
		A light that casts no shadows has its whole mask lit,
		so the passes adding the light its shadows removed would add nothing.
		Only what's known before the light jobs run decides it - they still run in parallel with this.
	*/

	auto casts_any_shadow = [](const auto& request) {
		return request.filter.maskBits != 0;
	};

	/*
		A light keeping its hue through walls adds to the hue light texture exactly what it adds to the removed light texture.
		So when all the shadow casting lights do, only the removed light texture is drawn, and it serves as both.
	*/

	const bool all_hue_through_walls = std::all_of(
		light_requests.begin(),
		light_requests.end(),
		[&](const auto& request) {
			return request.light_data.hue_through_walls || !casts_any_shadow(request);
		}
	);

	const bool hue_from_removed_light = 
		keep_hue_in_shadows
		&& all_hue_through_walls
		&& in.removed_light_fbo != nullptr
	;

	const bool draw_hue_light = keep_hue_in_shadows && !hue_from_removed_light;

	const bool legacy_bands = 
		!physical_lights
		&& cosm.get_common_significant().light.brighten_lights
	;

	const bool draw_removed_light = 
		in.removed_light_fbo != nullptr
		&& (legacy_bands || hue_from_removed_light)
	;

	auto set_pass = [&](const int pass) {
		set_uniform(light_shader, augs::common_uniform_name::light_pass, pass);
	};

	/*
		The mask is bound only while the passes reading it render elsewhere,
		so the light texture is never a sampler and a render target at once - radeonsi treats that as a feedback loop.
	*/

	auto bind_light_mask = [&]() {
		renderer.set_active_texture(6);
		in.light_fbo.get_texture().set_as_current(renderer);
		renderer.set_active_texture(0);
	};

	auto unbind_light_mask = [&]() {
		renderer.set_active_texture(6);
		augs::graphics::texture::set_current_to_none(renderer);
		renderer.set_active_texture(0);
	};

	auto overlay_light_polygons = [&]() {

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

				const auto max_distance = set_falloff_uniforms(light_shader, light_uniform, light.falloff, light.attenuation, variation_vals[0]);

				set_uniform(
					light_shader, 
					light_uniform.multiply_color,
					white.rgb()
				);

				/*
					Every light is its whole reach, scaled by the mask of its shadows
					built in the alpha of the light texture - see light.fsh for the passes.
					Lights with a height build it from their own shadows (light_height_shadows.h),
					the other ones from their visibility polygons.
				*/

				const auto reach = xywh::center_and_size(world_light_pos, request.queried_rect);

				const bool light_casts_shadow = casts_any_shadow(request);

				if (light.height > 0.0f) {
					set_pass(3);
					renderer.set_max_blending();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, white);

					set_pass(2);
					renderer.set_min_blending();
					renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_SHADOW_MASKS, i);

					if (draw_hue_light && !light.hue_through_walls && light_casts_shadow) {
						/*
							The mask holds only the obstacles reaching the ceiling now,
							so the light removed by the lower ones is taken only where walls let it through.
						*/

						in.hue_light_fbo->set_as_current(renderer);

						set_pass(9);
						renderer.set_color_only_additive_blending();
						bind_light_mask();
						renderer.call_triangles(augs::dedicated_buffer_vector::LIGHT_LOW_SHADOW_MASKS, i);
						unbind_light_mask();

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

				/*
					Lights quantized in the editor go in discrete bands unless all light is banded already.
					Muzzle flashes - whenever neons are quantized too.
				*/

				const bool quantized_flash = request.muzzle_flash && in.posterized_neon_shader != nullptr;
				const bool quantized = quantized_flash || (light.quantized && !legacy_bands);

				set_pass(quantized ? 11 : 4);
				renderer.set_dst_alpha_additive_blending();
				::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);

				if (draw_hue_light && light.hue_through_walls && light_casts_shadow) {
					/*
						The light keeps its hue through all its shadows, walls included.
					*/

					in.hue_light_fbo->set_as_current(renderer);

					set_pass(7);
					renderer.set_color_only_additive_blending();
					bind_light_mask();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);
					unbind_light_mask();

					in.light_fbo.set_as_current(renderer);
				}

				if (draw_removed_light && light_casts_shadow) {
					/*
						The light all the shadows removed - see draw_removed_light.
					*/

					in.removed_light_fbo->set_as_current(renderer);

					set_pass(7);
					renderer.set_color_only_additive_blending();
					bind_light_mask();
					::draw_light_area(renderer, reach, world_light_pos, max_distance, request.color);
					unbind_light_mask();

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
			set_uniform(wall_light_shader, wall_light_uniform.physical, physical_lights ? 1 : 0);
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

			const auto wall_light_aabb = xywh::center_and_size(world_light_pos, light.calc_wall_reach());

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

			set_falloff_uniforms(wall_light_shader, wall_light_uniform, light.wall_falloff, light.wall_attenuation, variation_vals[3]);

			set_uniform(
				wall_light_shader, 
				wall_light_uniform.multiply_color,
				light.color.rgb()
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

	/*
		Neon maps and the glow of electric armor, optionally in discrete bands - unless the legacy lights of the map already band all light.
		Missiles and particles stay smooth - their neons are stretched along trails.
	*/

	auto set_neon_shader = [&]() {
		if (in.posterized_neon_shader != nullptr) {
			in.posterized_neon_shader->set_as_current(renderer);
		}
		else {
			standard_shader.set_as_current(renderer);
		}
	};

	if (in.posterized_neon_shader != nullptr) {
		in.posterized_neon_shader->set_as_current(renderer);
		in.posterized_neon_shader->set_projection(renderer, in.cone.get_projection_matrix());
		set_uniform(*in.posterized_neon_shader, augs::common_uniform_name::ambient_color, cosm.get_common_significant().light.ambient_color);

		const bool smooth_brightening = physical_lights && cosm.get_common_significant().light.brighten_lights;
		set_uniform(*in.posterized_neon_shader, augs::common_uniform_name::smooth_brightening, smooth_brightening ? 1 : 0);
	}

	auto restore_renderer = [&]() {
		renderer.set_standard_blending();

		augs::graphics::fbo::set_current_to_marked(in.renderer);

		renderer.set_active_texture(2);
		in.light_fbo.get_texture().set_as_current(renderer);

		if (in.removed_light_fbo != nullptr) {
			renderer.set_active_texture(5);
			in.removed_light_fbo->get_texture().set_as_current(renderer);
		}

		if (hue_from_removed_light) {
			renderer.set_active_texture(7);
			in.removed_light_fbo->get_texture().set_as_current(renderer);
		}
		else if (in.hue_light_fbo != nullptr) {
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

	if (draw_removed_light) {
		in.removed_light_fbo->set_as_current(renderer);
		renderer.clear_current_fbo();
	}

	if (draw_hue_light) {
		in.hue_light_fbo->set_as_current(renderer);
		renderer.clear_current_fbo();
	}

	in.light_fbo.set_as_current(renderer);
	in.write_fow_to_stencil();

	renderer.set_clear_color(cosm.get_common_significant().light.ambient_color);
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	renderer.set_additive_blending();
	set_neon_shader();

	renderer.call_triangles(D::GROUND_NEONS);
	renderer.set_standard_blending();
	in.neon_occlusion_callback(false);
	renderer.call_and_clear_triangles();
	renderer.call_triangles(D::GROUND_NEON_OCCLUDERS);
	renderer.set_additive_blending();
	set_neon_shader();
	draw_sentience_neons();

	if (in.strict_fow) {
		renderer.stencil_positive_test();
		renderer.set_stencil(true);
	}

	renderer.call_triangles(D::DROPPED_ITEMS_NEONS);
	renderer.call_triangles(D::REMNANTS_NEONS);

	if (in.strict_fow) {
		renderer.set_stencil(false);
	}

	renderer.call_triangles(D::UNDER_FOREGROUND_NEONS);

	standard_shader.set_as_current(renderer);

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

	set_neon_shader();
	renderer.call_triangles(D::FOREGROUND_NEONS);
	renderer.call_triangles(D::LYING_CORPSES_NEONS);
	renderer.call_triangles(D::GROUND_DECALS_NEONS);

	standard_shader.set_as_current(renderer);
	renderer.call_and_clear_triangles();

	performance.num_drawn_lights.measure(num_lights);
	performance.num_drawn_wall_lights.measure(num_wall_lights);

	restore_renderer();
}