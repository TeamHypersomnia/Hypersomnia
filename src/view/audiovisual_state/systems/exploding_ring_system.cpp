#include "augs/misc/randomization.h"
#include "augs/templates/container_templates.h"

#include "game/cosmos/cosmos.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/exploding_ring_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"

void exploding_ring_system::clear() {
	rings.clear();
}

void exploding_ring_system::acquire_new_rings(const std::vector<exploding_ring_input>& new_rings) {
	rings.reserve(rings.size() + new_rings.size());

	for (const auto& r : new_rings) {
		ring n;
		n.in = r;
		n.time_of_occurence_seconds = global_time_seconds;
		rings.push_back(n);
	}
}

void exploding_ring_system::advance(
	randomization& rng,
	const cosmos& cosmos,
	const particle_effects_map& manager,
	const augs::delta dt,
	particles_simulation_system& particles_output_for_effects
) {
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

				const auto max_particles_to_spawn = static_cast<unsigned>(160.f * maximum_spawn_radius / 400.f);
				LOG_NVPS(maximum_spawn_radius, max_particles_to_spawn);
				const auto& common_assets = cosmos.get_common_assets();

				const auto* const ring_smoke = mapped_or_nullptr(manager, common_assets.exploding_ring_smoke);
				const auto* const ring_sparkles = mapped_or_nullptr(manager, common_assets.exploding_ring_sparkles);

				if (ring_smoke != nullptr && ring_sparkles != nullptr) {
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

						const auto particles_to_spawn = max_particles_to_spawn * particles_amount_ratio;

						for (auto p = 0u; p < particles_to_spawn; ++p) {
							const auto angular_translation_multiplier = p / static_cast<float>(particles_to_spawn);
							const auto spawn_particle_along_line = (tri[1] + along_edge * along_edge_length * angular_translation_multiplier) - r.center;
							const auto circle_radius = std::min(spawn_particle_along_line.length(), vis.source_square_side / 2);

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

void exploding_ring_system::draw_rings(
	const augs::drawer_with_default output,
	augs::special_buffer& specials,
	const cosmos&,
	const camera_cone cone
) const {
	const auto& eye = cone.eye;

	for (const auto& e : rings) {
		const auto& r = e.in;

		const auto passed = global_time_seconds - e.time_of_occurence_seconds;
		auto ratio = passed / r.maximum_duration_seconds;

		const auto inner_radius_now = augs::interp(r.inner_radius_start_value, r.inner_radius_end_value, ratio) / eye.zoom;
		const auto outer_radius_now = augs::interp(r.outer_radius_start_value, r.outer_radius_end_value, ratio) / eye.zoom;

		const auto world_explosion_center = r.center;

		augs::special sp;
		sp.v1 = cone.to_screen_space(world_explosion_center);
		sp.v1.y = cone.screen_size.y - sp.v1.y;

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
			output.aabb_centered(
				world_explosion_center, 
				vec2(outer_radius_now * 2, outer_radius_now * 2),
				considered_color
			);

			for (int s = 0; s < 6; ++s) {
				specials.push_back(sp);
			}
		}
	}
}

void exploding_ring_system::draw_highlights_of_rings(
	const augs::drawer output,
	const augs::atlas_entry highlight_tex,
	const cosmos&,
	const camera_cone cone
) const {
	for (const auto& r : rings) {
		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		auto ratio = passed / (r.in.maximum_duration_seconds * 1.2);

		const auto radius = std::max(r.in.outer_radius_end_value, r.in.outer_radius_start_value) / cone.eye.zoom;
		auto highlight_col = r.in.color;

		const auto highlight_amount = 1.f - ratio;

		if (highlight_amount > 0.f) {
			highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

			output.aabb_centered(
				highlight_tex,
				r.in.center,
				vec2(radius, radius),
				highlight_col
			);
		}
	}
}