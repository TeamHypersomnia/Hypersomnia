#include "augs/drawing/drawing.hpp"
#include "augs/misc/randomization.h"
#include "augs/templates/container_templates.h"

#include "game/cosmos/cosmos_common.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/exploding_ring_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/special_effects_settings.h"
#include "view/viewables/particle_types.hpp"

void exploding_ring_system::clear() {
	rings.clear();
}

void exploding_ring_system::advance(
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

		const auto secs_remaining = r.maximum_duration_seconds - (global_time_seconds - e.time_of_occurence_seconds);

		if (secs_remaining < 0.06f) {
			const auto& vis = r.visibility;

			if (r.emit_particles_on_ring && vis.get_num_triangles() > 0) {
				r.emit_particles_on_ring = false;
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

#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/typed_entity_handle.h"
#include "game/cosmos/entity_handle.h"

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
		const auto world_explosion_center = r.center;

		const auto passed = global_time_seconds - e.time_of_occurence_seconds;
		const auto ratio = passed / r.maximum_duration_seconds;

		const auto inner_radius_now = augs::interp(r.inner_radius_start_value, r.inner_radius_end_value, ratio) / eye.zoom;
		const auto outer_radius_now = augs::interp(r.outer_radius_start_value, r.outer_radius_end_value, ratio) / eye.zoom;

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
		considered_color.a = static_cast<rgba_channel>(considered_color.a * (1.f - ratio));

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
		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		const auto ratio = passed / (r.in.maximum_duration_seconds * 1.2);

		const auto radius = std::max(r.in.outer_radius_end_value, r.in.outer_radius_start_value);
		auto highlight_col = r.in.color;

		const auto highlight_amount = 1.f - ratio;

		const auto aabb_size = vec2::square(radius * 2);
		const auto explosion_ltrb = ltrbi::center_and_size(r.in.center, aabb_size);

		if (!queried_camera_aabb.hover(explosion_ltrb)) {
			continue;
		}

		if (highlight_amount > 0.f) {
			highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

			output.aabb(
				highlight_tex,
				explosion_ltrb,
				highlight_col
			);
		}
	}
}