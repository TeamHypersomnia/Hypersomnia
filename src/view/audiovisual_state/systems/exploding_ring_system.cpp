#include <cstddef>
#include <cmath>
#include <limits>
#include "augs/drawing/drawing.hpp"
#include "augs/misc/randomization.h"
#include "augs/templates/container_templates.h"

#include "game/cosmos/cosmos_common.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/typed_entity_handle.h"
#include "game/cosmos/entity_handle.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/exploding_ring_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/special_effects_settings.h"
#include "view/viewables/particle_types.hpp"

/*
	The radius of a force grenade's explosion.
	An explosion this big spawns the whole max_particles_per_explosion budget.
	Bigger explosions scale their particles up instead of spawning more of them,
	smaller ones spawn proportionally fewer particles, keeping the density.
*/

constexpr auto reference_explosion_radius = 380.f;

static const auto& get_particles_look(const exploding_ring_input& r, const explosions_settings& settings) {
	return r.explosion_particles.fire ? settings.fire_particles : settings.standard_particles;
}

static auto get_particles_duration_secs(const exploding_ring_input& r) {
	return std::max(r.maximum_duration_seconds, 0.01f);
}

static auto get_particles_lifetime_mult(const exploding_ring_input& r) {
	return std::max(0.01f, r.explosion_particles.lifetime_mult);
}

/*
	The particles - and the explosion's light - go through the whole palette
	over particle_palette_duration_fraction of the ring's duration, then stay in its last color.
*/

static auto calc_palette_step_ms(const exploding_ring_input& r, const explosion_particles_settings& look) {
	const auto& palette = r.explosion_particles.palette;

	if (palette.size() < 2) {
		return 1.f;
	}

	const auto palette_duration_ms = 
		get_particles_duration_secs(r) * 1000.f 
		* get_particles_lifetime_mult(r) 
		* look.particle_palette_duration_fraction
	;

	return std::max(1.f, palette_duration_ms / static_cast<float>(palette.size() - 1));
}

static void spawn_explosion_particles(
	const exploding_ring_input& r,
	randomization& rng,
	const explosions_settings& settings,
	const particles_emission& emission,
	particles_simulation_system& particles
) {
	const auto& definitions = emission.get_definitions<general_particle>();
	const auto& def = r.explosion_particles;
	const auto& look = ::get_particles_look(r, settings);

	if (definitions.empty()) {
		return;
	}

	const auto max_ring_radius = std::max(r.outer_radius_start_value, r.outer_radius_end_value);
	const auto radius_ratio = max_ring_radius / reference_explosion_radius;
	const auto size_mult = std::max(1.f, radius_ratio);
	const auto amount_mult = std::min(1.f, radius_ratio * radius_ratio);

	/*
		Each of the explosion's two rings takes half of the budget.
	*/
	const auto total_to_spawn = 
		static_cast<float>(settings.max_particles_per_explosion) 
		* 0.5f 
		* amount_mult
	;

	const auto duration_secs = ::get_particles_duration_secs(r);
	const auto duration_ms = duration_secs * 1000.f;
	const auto explosion_lifetime_mult = ::get_particles_lifetime_mult(r);
	const auto max_lifetime_ms = duration_ms * look.particle_lifetime_mult_max * explosion_lifetime_mult;

	const auto inner_start = r.inner_radius_start_value;
	const auto outer_start = r.outer_radius_start_value;
	const auto band_width = outer_start - inner_start;

	/*
		Both edges of the ring move at constant speeds over its duration.
		Each particle moves at the speed interpolated between them by where it starts within the ring,
		so the particles keep filling the ring as it expands or contracts.
	*/

	const auto inner_vel = (r.inner_radius_end_value - inner_start) / duration_secs;
	const auto outer_vel = (r.outer_radius_end_value - outer_start) / duration_secs;

	const auto spin = rng.randval(0, 1) == 0 ? -1.f : 1.f;

	/*
		The ring's duration is already shortened by the playback speed, and so are all the timings relative to it.
		The absolute speeds and accelerations are scaled so that the paths stay the same, only traced faster.
	*/

	const auto playback_speed = std::max(0.01f, def.playback_speed);

	const auto variation = std::max(0.f, def.variation);
	const auto jitter_radius = look.particle_jitter_radius * variation;
	const auto jitter_degrees = look.particle_jitter_degrees * variation;
	const auto random_acceleration_min = look.particle_random_acceleration_min * variation * playback_speed * playback_speed;
	const auto random_acceleration_max = look.particle_random_acceleration_max * variation * playback_speed * playback_speed;

	/*
		Less variation narrows the lifetime range towards the moment the ring ends (1),
		so the particles die out more in unison - right as they arrive at the ring's end radius,
		not lingering there after having eased out to a stop.
	*/

	const auto lifetime_narrowing_target = std::clamp(1.f, look.particle_lifetime_mult_min, look.particle_lifetime_mult_max);
	const auto clamped_variation = std::min(1.f, variation);
	const auto lifetime_mult_min = augs::interp(lifetime_narrowing_target, look.particle_lifetime_mult_min, clamped_variation);
	const auto lifetime_mult_max = augs::interp(lifetime_narrowing_target, look.particle_lifetime_mult_max, clamped_variation);

	/*
		The exponential slowdown with the time constant tau covers tau * (1 - exp(-duration / tau)) 
		in the time a constant speed covers the whole duration - the initial speeds are compensated by the ratio,
		so the particles still reach the ring's end radius right when the ring ends.
	*/

	const auto ease_tau_secs = look.particle_ease_out * std::max(0.f, def.ease_out_mult) * duration_secs;
	const auto velocity_damping = ease_tau_secs > 0.f ? 1.f / ease_tau_secs : 0.f;

	const auto ease_compensation = 
		ease_tau_secs > 0.f ?
		duration_secs / (ease_tau_secs * (1.f - std::exp(-duration_secs / ease_tau_secs))) :
		1.f
	;

	/*
		Opaque colors only - the particles are pixel art, no translucency.
	*/

	auto ring_color = r.color;
	ring_color.a = 255;

	/*
		Hot means the explosion's inner ring color, slightly brightened -
		the same for both rings, so that the blast starts in a single color.
	*/

	auto hot_color = r.explosion_particles_hot_color;

	{
		auto hot_hsl = hot_color.get_hsl();
		hot_hsl.l += (1.f - hot_hsl.l) * 0.35f;
		hot_color.set_hsl(hot_hsl);
	}

	hot_color.a = 255;

	auto cool_color = ring_color;

	if (def.cool_color.a > 0) {
		cool_color = def.cool_color;
	}
	else if (const auto cooling = std::clamp(def.cooling, 0.f, 1.f); cooling > 0.f) {
		cool_color.mult_brightness(augs::interp(1.f, look.particle_cool_brightness, cooling));
	}

	cool_color.a = 255;

	/*
		Only the fiery (red to orange) explosions leave embers behind -
		the others (white, cyan etc.) are luminous, cybernetic blasts.
	*/

	const auto ember_fraction = [&]() {
		const auto ring_hsl = ring_color.get_hsl();
		const bool is_fiery = ring_hsl.s > 0.5f && (ring_hsl.h <= 45 || ring_hsl.h >= 345);

		return is_fiery ? look.particle_ember_fraction : 0.f;
	}();

	const auto palette_step_ms = ::calc_palette_step_ms(r, look);

	const auto hot_until_ms = duration_ms * look.particle_hot_fraction;
	const auto cool_from_ms = duration_ms * look.particle_cool_from_fraction * std::max(0.f, def.cool_from_mult);
	const auto cool_until_ms = cool_from_ms + std::max(1.f, duration_ms * look.particle_cool_duration_fraction);

	auto spawn_one = [&](const float angle, const float max_particle_radius) {
		/* Uniform density over the ring's area. */
		const auto radius = std::sqrt(augs::interp(inner_start * inner_start, outer_start * outer_start, rng.randval(0.f, 1.f)));
		const auto t = std::abs(band_width) > 0.001f ? (radius - inner_start) / band_width : 0.5f;
		const auto jittered_radius = std::max(0.f, radius + rng.randval_h(jitter_radius * size_mult));

		if (jittered_radius > max_particle_radius) {
			return;
		}

		auto p = explosion_particle();

		const bool is_ember = rng.randval(0.f, 1.f) < ember_fraction;

		p.sprite = definitions[rng.randval(0u, static_cast<unsigned>(definitions.size()) - 1)];
		p.sprite.multiply_size(size_mult * look.particle_size_mult * (is_ember ? 0.5f : 1.f));
		p.sprite.max_lifetime_ms = duration_ms * explosion_lifetime_mult * rng.randval(lifetime_mult_min, lifetime_mult_max);
		p.sprite.shrink_when_ms_remaining = max_lifetime_ms;

		p.center = r.center;
		p.radius = jittered_radius;
		p.radial_vel = augs::interp(inner_vel, outer_vel, t) * ease_compensation;
		p.tangential_vel = spin * size_mult * playback_speed * rng.randval(look.particle_tangential_speed_min, look.particle_tangential_speed_max) * ease_compensation;
		p.angle = angle;
		p.rotation_offset = rng.randval_h(jitter_degrees);
		p.max_radius = max_particle_radius;
		p.velocity_damping = velocity_damping;

		p.drift_acc = 
			vec2::from_degrees(rng.randval(0.f, 360.f)) 
			* size_mult
			* rng.randval(random_acceleration_min, random_acceleration_max)
		;

		if (is_ember) {
			/*
				Embers linger after the blast, drifting around more erratically,
				shrinking over their whole long lifetime.
			*/
			p.sprite.max_lifetime_ms *= look.particle_ember_lifetime_mult;
			p.sprite.shrink_when_ms_remaining = p.sprite.max_lifetime_ms;
			p.radial_vel *= 0.5f;
			p.tangential_vel *= 0.5f;
			p.drift_acc *= 2.f;
		}

		p.hot_color = hot_color;
		p.ring_color = ring_color;
		p.cool_color = cool_color;
		p.hot_until_ms = hot_until_ms;
		p.cool_from_ms = cool_from_ms;
		p.cool_until_ms = cool_until_ms;

		if (!def.palette.empty()) {
			p.palette = def.palette;
			p.palette_offset = r.explosion_particles_palette_offset;
			p.palette_step_ms = palette_step_ms;
		}

		p.update_sprite_transform();
		p.update_sprite_color();
		particles.add_explosion_particle(p);
	};

	const auto& vis = r.visibility;
	const auto num_triangles = vis.get_num_triangles();

	if (num_triangles == 0) {
		for (auto i = 0.f; i < total_to_spawn; i += 1.f) {
			spawn_one(rng.randval(0.f, 360.f), std::numeric_limits<float>::max());
		}

		return;
	}

	/*
		The visibility triangles fan out from the center.
		Each spawns its share of the particles in its angular sector,
		only up to the sector's far edge - so the walls occlude the explosion,
		and the particles die upon reaching them.
	*/

	auto carried_amount = 0.f;

	for (std::size_t i = 0; i < num_triangles; ++i) {
		const auto tri = vis.get_world_triangle(i, r.center);
		const auto first_dir = tri[1] - r.center;
		const auto second_dir = tri[2] - r.center;
		const auto first_angle = first_dir.degrees();
		const auto sweep = first_dir.full_degrees_between(second_dir);

		if (!std::isfinite(sweep) || !std::isfinite(first_angle)) {
			continue;
		}

		const auto far_edge = tri[2] - tri[1];
		const auto fallback_boundary = std::max(first_dir.length(), second_dir.length());

		carried_amount += total_to_spawn * std::abs(sweep) / 360.f;

		while (carried_amount >= 1.f) {
			carried_amount -= 1.f;

			const auto angle = first_angle + sweep * rng.randval(0.f, 1.f);
			const auto dir = vec2::from_degrees(angle);
			const auto denom = dir.cross(far_edge);

			const auto boundary = 
				std::abs(denom) > 0.0001f ? 
				first_dir.cross(far_edge) / denom :
				fallback_boundary
			;

			spawn_one(angle, boundary);
		}
	}
}

void exploding_ring_system::clear() {
	rings.clear();
}

void exploding_ring_system::advance(
	const cosmos& cosm,
	randomization& rng,
	const camera_cone queried_cone,
	const common_assets& common,
	const particle_effects_map& manager,
	const augs::delta dt,
	const explosions_settings& settings,
	particles_simulation_system& particles_output_for_effects
) {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();

	auto& particles = particles_output_for_effects;

	global_time_seconds += dt.in_seconds();

	erase_if(rings, [&](ring& e) {
		auto& r = e.in;

		const auto& look = ::get_particles_look(r, settings);

		e.palette_step_ms = ::calc_palette_step_ms(r, look);
		e.light_brightness_mult = look.light_brightness_mult;

		if (r.is_explosion_thin_ring) {
			r.fixed_thickness = look.thin_ring_thickness * r.explosion_particles.thin_ring_thickness_mult;
		}

		if (r.target.is_set()) {
			if (const auto handle = cosm[r.target]) {
				r.center = handle.get_logic_transform().pos;
			}
		}

		if (r.emit_explosion_particles) {
			r.emit_explosion_particles = false;

			const auto max_ring_radius = std::max(r.outer_radius_start_value, r.outer_radius_end_value);
			const bool visible = queried_camera_aabb.hover(ltrb::center_and_size(r.center, vec2::square(max_ring_radius * 2)));

			if (visible) {
				if (const auto* const effect = mapped_or_nullptr(manager, common.exploding_ring_explosion_particles)) {
					if (!effect->emissions.empty()) {
						::spawn_explosion_particles(r, rng, settings, effect->emissions[0], particles);
					}
				}
			}
		}

		const auto secs_remaining = r.maximum_duration_seconds - (global_time_seconds - e.time_of_occurence_seconds);

		if (secs_remaining < 0.06f) {
			const auto& vis = r.visibility;

			if (r.emit_ring_end_particles && vis.get_num_triangles() > 0) {
				r.emit_ring_end_particles = false;
				const auto minimum_spawn_radius = std::min(r.outer_radius_start_value, r.outer_radius_end_value);
				const auto maximum_spawn_radius = std::max(r.outer_radius_start_value, r.outer_radius_end_value);
				const auto spawn_radius_width = (maximum_spawn_radius - minimum_spawn_radius) / 2.4f;

				const bool visible = queried_camera_aabb.hover(ltrb::center_and_size(r.center, vec2::square(maximum_spawn_radius * 2)));

				const auto max_particles_to_spawn = static_cast<unsigned>(160.f * maximum_spawn_radius / 400.f) * settings.sparkle_amount;

				const auto* const ring_smoke = mapped_or_nullptr(manager, common.exploding_ring_smoke);
				const auto* const ring_sparkles = mapped_or_nullptr(manager, common.exploding_ring_sparkles);

				if (visible && ring_smoke != nullptr && ring_sparkles != nullptr) {
					auto smokes_emission = ring_smoke->emissions.at(0);
					smokes_emission.target_layer = particle_layer::DIM_SMOKES;
					const auto& sparkles_emission = ring_sparkles->emissions.at(0);

					for (auto i = 0u; i < vis.get_num_triangles(); ++i) {
						const auto tri = vis.get_world_triangle(i, r.center);
						const auto edge_v1 = tri[1] - tri[0];
						const auto edge_v2 = tri[2] - tri[0];
						const auto along_edge_length = (tri[2] - tri[1]).length();
						const auto along_edge = (tri[2] - tri[1]) / along_edge_length;

						/* 
							Since these are edges of a triangle,
						   	the angle must be less than 180, so we use simple "degrees between" 
						*/

						const auto angular_translation = edge_v1.degrees_between(edge_v2);
						const auto particles_amount_ratio = angular_translation / 360.f;

						const auto sparkles_to_spawn = static_cast<float>(max_particles_to_spawn) * particles_amount_ratio * settings.sparkle_amount;
						const auto smokes_to_spawn = static_cast<float>(max_particles_to_spawn) * particles_amount_ratio * settings.smoke_amount;

						for (auto p = 0u; p < sparkles_to_spawn; ++p) {
							const auto angular_translation_multiplier = p / sparkles_to_spawn;
							const auto spawn_particle_along_line = (tri[1] + along_edge * along_edge_length * angular_translation_multiplier) - r.center;
							const auto circle_radius = std::min(spawn_particle_along_line.length(), vis.source_queried_rect.x / 2);

							{
								const auto spawner = [&](auto dummy) {
									if (sparkles_emission.has<decltype(dummy)>()) {
										auto new_p = particles.spawn_particle<decltype(dummy)>(
											rng,
											0.f,
											{ 200.f, 220.f },
											r.center + vec2(spawn_particle_along_line).set_length(
												std::max(1.f, circle_radius - rng.randval(0.f, spawn_radius_width))
											),
											0.f,
											360.f,
											sparkles_emission
										);

										new_p.colorize(r.color.rgb());
										new_p.acc /= 2;
										new_p.linear_damping /= 2;
										new_p.acc.rotate(rng.randval(0.f, 360.f));

										particles.add_particle(sparkles_emission.target_layer, new_p);
										//new_p.max_lifetime_ms *= 1.4f;
									}
								};

								spawner(animated_particle());
								spawner(general_particle());
							}
						}

						for (auto p = 0u; p < smokes_to_spawn; ++p) {
							const auto angular_translation_multiplier = p / smokes_to_spawn;
							const auto spawn_particle_along_line = (tri[1] + along_edge * along_edge_length * angular_translation_multiplier) - r.center;
							const auto circle_radius = std::min(spawn_particle_along_line.length(), vis.source_queried_rect.x / 2);

							if (smokes_emission.has<general_particle>()) {
								auto new_p = particles.spawn_particle<general_particle>(
									rng,
									0.f,
									{ 100.f, 120.f },
									r.center + vec2(spawn_particle_along_line).set_length(
										std::max(1.f, circle_radius - rng.randval(0.f, spawn_radius_width))
									),
									0.f,
									360.f,
									smokes_emission
								);

								new_p.color.set_rgb(r.color.rgb());
								new_p.color.a *= 2;

								particles.add_particle(smokes_emission.target_layer, new_p);

								//new_p.acc /= 2;
								//new_p.acc.rotate(rng.randval(0.f, 360.f));
								//new_p.max_lifetime_ms *= 1.4f;
							}
						}
					}
				}

			}
		}

		return secs_remaining <= 0.f;
	});
}

void exploding_ring_system::draw_continuous_rings(
	const cosmos& cosm,
	const augs::drawer_with_default output,
	augs::special_buffer& specials,
	const camera_cone queried_cone,
	const camera_cone actual_cone
) const {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();
	const auto& eye = actual_cone.eye;

	const auto sane_default_ratio = 0.5f;

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_portal_handle) {
			const auto& portal = typed_portal_handle.template get<components::portal>();

			if (!portal.rings_effect.is_enabled) {
				return;
			}

			const auto& e = portal.rings_effect.value;

			if (const auto aabb = typed_portal_handle.find_aabb()) {
				if (!queried_camera_aabb.hover(*aabb)) {
					return;
				}
			}

			const auto passed = sane_default_ratio * global_time_seconds * e.effect_speed;

			const auto radius = typed_portal_handle.get_logical_size().smaller_side() / 2;
			const auto ring_center = typed_portal_handle.get_logic_transform().pos;

			auto draw_rings_with = [&](
				const auto inner_start,
				const auto inner_end,
				const auto outer_start,
				const auto outer_end,
				auto color,
				const auto speed_mult,
				const float min_alpha = 0.0f,
				const float max_alpha = 0.8f
			) {
				const auto ratio = float(std::sin(2 * PI<float> * passed * speed_mult) + 1) / 2;

				const auto inner_radius_now = augs::interp(inner_start, inner_end, ratio) / eye.zoom;
				const auto outer_radius_now = augs::interp(outer_start, outer_end, ratio) / eye.zoom;

				color.a = static_cast<rgba_channel>(color.a * std::clamp(1.f - ratio, min_alpha, max_alpha));

				const auto aabb_size = vec2::square(outer_radius_now * 2 * eye.zoom);
				const auto ring_ltrb = ltrbi::center_and_size(ring_center, aabb_size);

				augs::special sp;

				sp.v1 = actual_cone.to_screen_space(ring_center);
				sp.v1.y = actual_cone.screen_size.y - sp.v1.y;

				sp.v2.x = inner_radius_now * eye.zoom * eye.zoom;
				sp.v2.y = outer_radius_now * eye.zoom * eye.zoom;

				output.aabb(
					ring_ltrb,
					color
				);

				for (int s = 0; s < 6; ++s) {
					specials.push_back(sp);
				}
			};

			draw_rings_with(0.0f, radius/2.f, radius / 2, radius / 1.2f, e.inner_color, 1.0f, 0.05f );
			draw_rings_with(0.0f, 0.0f, radius / 2, radius / 1.2f, e.inner_color, 0.4212f, 0.05f);

			draw_rings_with(radius / 1.5f, radius / 2.0f, radius, radius / 1.2f, e.outer_color, 1.243f, 0.0f, 0.5f);
			draw_rings_with(0.0f, radius/10.0f, radius, radius, e.outer_color, 1.0f, 0.3f, 0.6f);
		}
	);
}

void exploding_ring_system::draw_rings(
	const augs::drawer_with_default output,
	augs::special_buffer& specials,
	const camera_cone queried_cone,
	const camera_cone actual_cone
) const {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();
	const auto& eye = actual_cone.eye;

	for (const auto& e : rings) {
		const auto& r = e.in;

		if (!r.draw_color_rings) {
			continue;
		}

		const auto world_explosion_center = r.center;

		const auto passed = global_time_seconds - e.time_of_occurence_seconds;
		const auto ratio = passed / r.maximum_duration_seconds;

		float outer_radius_now;

		if (r.halve_per_ms > 0.0f) {
			const auto passed_ms = static_cast<float>(passed) * 1000.0f;
			const auto exp_ratio = 1.0f - std::pow(0.5f, passed_ms / r.halve_per_ms);
			outer_radius_now = augs::interp(r.outer_radius_start_value, r.outer_radius_end_value, exp_ratio) / eye.zoom;
		}
		else {
			outer_radius_now = augs::interp(r.outer_radius_start_value, r.outer_radius_end_value, ratio) / eye.zoom;
		}

		float inner_radius_now;

		if (r.fixed_thickness > 0.0f) {
			const auto thickness_now = 
				r.is_explosion_thin_ring ? 
				r.fixed_thickness * std::max(0.f, 1.f - static_cast<float>(ratio)) :
				r.fixed_thickness
			;

			inner_radius_now = outer_radius_now - thickness_now / eye.zoom;
		}
		else {
			inner_radius_now = augs::interp(r.inner_radius_start_value, r.inner_radius_end_value, ratio) / eye.zoom;
		}

		const auto aabb_size = vec2::square(outer_radius_now * 2 * eye.zoom);
		const auto explosion_ltrb = ltrbi::center_and_size(world_explosion_center, aabb_size);

		if (!queried_camera_aabb.hover(explosion_ltrb)) {
			continue;
		}

		augs::special sp;
		sp.v1 = actual_cone.to_screen_space(world_explosion_center);
		sp.v1.y = actual_cone.screen_size.y - sp.v1.y;

		sp.v2.x = inner_radius_now * eye.zoom * eye.zoom;
		sp.v2.y = outer_radius_now * eye.zoom * eye.zoom;

		const auto& vis = r.visibility;

		auto considered_color = r.color;
		const float alpha_t = augs::interp(1.0f, r.final_alpha, ratio);
		considered_color.a = static_cast<rgba_channel>(considered_color.a * alpha_t);

		const int alpha_levels = 2;
		const int alpha_step = 255 / alpha_levels;

		float a = alpha_t;
		int a255 = int(a * 255.0f);

		a255 = alpha_step * (a255 / alpha_step) + alpha_step;

		/*
			Rounding up from full alpha would overshoot 255 and wrap around the channel.
		*/
		a = std::min(1.0f, float(a255) / 255.0f);

		considered_color.a = static_cast<rgba_channel>(
			considered_color.a * a
		);

		if (vis.get_num_triangles() > 0) {
			for (size_t t = 0; t < vis.get_num_triangles(); ++t) {
				const auto world_light_tri = vis.get_world_triangle(t, world_explosion_center);
				augs::vertex_triangle renderable_tri;

				renderable_tri.vertices[0].pos = world_light_tri[0];
				renderable_tri.vertices[1].pos = world_light_tri[1];
				renderable_tri.vertices[2].pos = world_light_tri[2];

				renderable_tri.vertices[0].color = considered_color;
				renderable_tri.vertices[1].color = considered_color;
				renderable_tri.vertices[2].color = considered_color;

				output.push(renderable_tri);

				for (int s = 0; s < 3; ++s) {
					specials.push_back(sp);
				}
			}
		}
		else {
			output.aabb(
				explosion_ltrb,
				considered_color
			);

			for (int s = 0; s < 6; ++s) {
				specials.push_back(sp);
			}
		}
	}
}

void exploding_ring_system::draw_highlights_of_continuous_rings(
	const cosmos& cosm,
	const augs::drawer output,
	const augs::atlas_entry highlight_tex,
	const camera_cone queried_cone
) const {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_portal_handle) {
			const auto& portal = typed_portal_handle.template get<components::portal>();

			const auto& e = portal;

			if (e.light_size_mult == 0.0f) {
				return;
			}

			const auto radius = e.light_size_mult * typed_portal_handle.get_logical_size().smaller_side() / 2;
			const auto ring_center = typed_portal_handle.get_logic_transform().pos;

			const auto aabb_size = vec2::square(radius * 2);
			const auto highlight_ltrb = ltrbi::center_and_size(ring_center, aabb_size);
			const auto highlight_col = e.light_color;

			if (!queried_camera_aabb.hover(highlight_ltrb)) {
				return;
			}

			output.aabb(
				highlight_tex,
				highlight_ltrb,
				highlight_col
			);
		}
	);
}

void exploding_ring_system::draw_highlights_of_explosions(
	const augs::drawer output,
	const augs::atlas_entry highlight_tex,
	const camera_cone queried_cone
) const {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();

	for (const auto& r : rings) {
		if (!r.in.emit_light) {
			continue;
		}

		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		const auto ratio = passed / (r.in.maximum_duration_seconds * 1.2);

		const auto radius = std::max(r.in.outer_radius_end_value, r.in.outer_radius_start_value);

		/*
			With a palette, the light goes through it in sync with the ring's particles.
		*/

		const auto& palette = r.in.explosion_particles.palette;

		auto highlight_col = 
			palette.empty() ? 
			r.in.color : 
			::sample_explosion_particles_palette(
				palette, 
				r.in.explosion_particles_palette_offset + static_cast<float>(passed * 1000.0) / r.palette_step_ms
			)
		;

		const auto highlight_amount = 1.f - ratio;

		const auto aabb_size = vec2::square(radius * 2);
		const auto explosion_ltrb = ltrbi::center_and_size(r.in.center, aabb_size);

		if (!queried_camera_aabb.hover(explosion_ltrb)) {
			continue;
		}

		if (highlight_amount > 0.f) {
			highlight_col.a = static_cast<rgba_channel>(std::min(255.0, 255 * highlight_amount * r.light_brightness_mult));

			output.aabb(
				highlight_tex,
				explosion_ltrb,
				highlight_col
			);
		}
	}
}