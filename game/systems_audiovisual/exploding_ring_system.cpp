#include "exploding_ring_system.h"
#include "augs/templates/container_templates.h"
#include "game/resources/manager.h"
#include "game/systems_audiovisual/particles_simulation_system.h"
#include "game/view/viewing_step.h"
#include "game/detail/particle_types.h"
#include "game/detail/camera_cone.h"
#include "augs/graphics/renderer.h"

void exploding_ring_system::acquire_new_rings(const std::vector<messages::exploding_ring>& new_rings) {
	rings.reserve(rings.size() + new_rings.size());

	for (const auto& r : new_rings) {
		ring n;
		n.in = r;
		n.time_of_occurence_seconds = global_time_seconds;
		rings.push_back(n);
	}
}

void exploding_ring_system::advance(
	const augs::delta dt,
	particles_simulation_system& particles_output_for_effects
) {
	auto& particles = particles_output_for_effects;

	global_time_seconds += dt.in_seconds();
	auto rng = fast_randomization(static_cast<size_t>(global_time_seconds * 10000));

	erase_remove(rings, [&](ring& e) {
		auto& r = e.in;

		const auto secs_remaining = r.maximum_duration_seconds - (global_time_seconds - e.time_of_occurence_seconds);

		if (secs_remaining < 0.06f) {
			const auto& vis = r.visibility;

			if (r.emit_particles_on_ring && vis.get_num_triangles() > 0) {
				r.emit_particles_on_ring = false;
				const auto minimum_spawn_radius = std::min(r.outer_radius_start_value, r.outer_radius_end_value);
				const auto maximum_spawn_radius = std::max(r.outer_radius_start_value, r.outer_radius_end_value);
				const auto spawn_radius_width = (maximum_spawn_radius - minimum_spawn_radius) / 2.4;

				const unsigned max_particles_to_spawn = 160;
				auto smokes_emission = get_resource_manager().find(assets::particle_effect_id::CAST_SPARKLES)->at(0);
				smokes_emission.particle_render_template.layer = render_layer::DIM_SMOKES;
				const auto& sparkles_emission = get_resource_manager().find(assets::particle_effect_id::CAST_SPARKLES)->at(1);

				for (auto i = 0u; i < vis.get_num_triangles(); ++i) {
					const auto tri = vis.get_world_triangle(i, r.center);
					const auto edge_v1 = tri[1] - tri[0];
					const auto edge_v2 = tri[2] - tri[0];
					const auto along_edge_length = (tri[2] - tri[1]).length();
					const auto along_edge = (tri[2] - tri[1]) / along_edge_length;

					const auto angular_translation = edge_v1.degrees_between(edge_v2);
					const auto particles_amount_ratio = angular_translation / 360.f;

					const auto particles_to_spawn = max_particles_to_spawn * particles_amount_ratio;

					for (auto p = 0u; p < particles_to_spawn; ++p) {
						const auto angular_translation_multiplier = p / static_cast<float>(particles_to_spawn);
						const auto spawn_particle_along_line = (tri[1] + along_edge * along_edge_length * angular_translation_multiplier) - r.center;
						const auto circle_radius = std::min(spawn_particle_along_line.length(), vis.source_square_side / 2);

						{
							const auto spawner = [&](auto dummy) {
								auto& new_p = particles.spawn_particle<decltype(dummy)>(
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
								new_p.acc.rotate(rng.randval(0.f, 360.f), vec2{ 0, 0 });

								particles.add_particle(sparkles_emission.particle_render_template.layer, new_p);
								//new_p.max_lifetime_ms *= 1.4f;
							};

							spawner(animated_particle());
							spawner(general_particle());
						}

						{
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

							new_p.face.color.rgb() = r.color.rgb();
							new_p.face.color.a *= 2;

							particles.add_particle(smokes_emission.particle_render_template.layer, new_p);

							//new_p.acc /= 2;
							//new_p.acc.rotate(rng.randval(0.f, 360.f), vec2{ 0, 0 });
							//new_p.max_lifetime_ms *= 1.4f;
						}
					}
				}
			}
		}

		return secs_remaining <= 0.f;
	});
}

void exploding_ring_system::draw_rings(
	augs::vertex_triangle_buffer& triangles,
	augs::special_buffer& specials,
	const camera_cone camera,
	const cosmos& cosmos
) const {
	for (const auto& e : rings) {
		const auto& r = e.in;

		const auto passed = global_time_seconds - e.time_of_occurence_seconds;
		auto ratio = passed / r.maximum_duration_seconds;

		const auto inner_radius_now = augs::interp(r.inner_radius_start_value, r.inner_radius_end_value, ratio);
		const auto outer_radius_now = augs::interp(r.outer_radius_start_value, r.outer_radius_end_value, ratio);

		const auto screen_space_center = camera.get_screen_space_revert_y(r.center);

		augs::special sp;
		sp.v1 = screen_space_center;
		sp.v2.x = inner_radius_now;
		sp.v2.y = outer_radius_now;

		const auto world_explosion_center = r.center;

		const auto& vis = r.visibility;

		auto considered_color = r.color;
		considered_color.a = static_cast<rgba_channel>(considered_color.a * (1.f - ratio));

		if (vis.get_num_triangles() > 0) {
			for (size_t t = 0; t < vis.get_num_triangles(); ++t) {
				const auto world_light_tri = vis.get_world_triangle(t, world_explosion_center);
				augs::vertex_triangle renderable_tri;

				renderable_tri.vertices[0].pos = camera[world_light_tri[0]];
				renderable_tri.vertices[1].pos = camera[world_light_tri[1]];
				renderable_tri.vertices[2].pos = camera[world_light_tri[2]];

				renderable_tri.vertices[0].color = considered_color;
				renderable_tri.vertices[1].color = considered_color;
				renderable_tri.vertices[2].color = considered_color;

				triangles.push_back(renderable_tri);

				for (int s = 0; s < 3; ++s) {
					specials.push_back(sp);
				}
			}
		}
		else {
			components::sprite spr;
			spr.set(assets::game_image_id::BLANK);
			spr.size.set(outer_radius_now * 2, outer_radius_now * 2);
			spr.color = considered_color;

			components::sprite::drawing_input in(triangles);
			in.renderable_transform.rotation = 0.f;
			in.renderable_transform.pos = world_explosion_center;
			in.camera = camera;

			spr.draw(in);

			for (int s = 0; s < 6; ++s) {
				specials.push_back(sp);
			}
		}
	}
}

void exploding_ring_system::draw_highlights_of_rings(
	augs::vertex_triangle_buffer& triangles,
	const camera_cone camera,
	const cosmos& cosmos
) const {
	for (const auto& r : rings) {
		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		auto ratio = passed / r.in.maximum_duration_seconds;

		const auto radius = std::max(r.in.outer_radius_end_value, r.in.outer_radius_start_value);
		auto highlight_col = r.in.color;

		const auto highlight_amount = 1.f - ratio;

		if (highlight_amount > 0.f) {
			components::sprite::drawing_input highlight(triangles);
			highlight.camera = camera;
			highlight.renderable_transform.pos = r.in.center;

			highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

			components::sprite spr;
			spr.set(assets::game_image_id::CAST_HIGHLIGHT, highlight_col);
			spr.size.set(radius, radius);

			spr.draw(highlight);
		}
	}
}