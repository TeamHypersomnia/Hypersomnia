#include "augs/misc/randomization.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/explosions.h"
#include "game/assets/ids/sound_buffer_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/messages/exploding_ring_input.h"
#include "game/messages/damage_message.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/enums/filters.h"
#include "game/components/sentience_component.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/debug_drawing_settings.h"
#include "game/messages/thunder_input.h"
#include "game/transcendental/data_living_one_step.h"

void standard_explosion_input::instantiate(
	const logic_step step,
	const components::transform explosion_location,
	const entity_id subject_if_any
) const {
	if (create_thunders_effect) {
		for (int t = 0; t < 4; ++t) {
			static randomization rng;
			thunder_input th;

			th.delay_between_branches_ms = {10.f, 25.f};
			th.max_branch_lifetime_ms = {40.f, 65.f};
			th.branch_length = {10.f, 120.f};

			th.max_all_spawned_branches = 40 + (t+1)*10;
			th.max_branch_children = 2;

			th.first_branch_root = explosion_location;
			th.first_branch_root.pos += rng.random_point_in_circle(70.f);
			th.first_branch_root.rotation += t * 360/4;
			th.branch_angle_spread = 40.f;

			th.color = t % 2 ? cyan : turquoise;

			step.post_message(th);
		}
	}

	sound_effect_input effect;
	effect.id = sound_effect; 
	effect.modifier.gain = sound_gain;

	effect.start(
		step,
		sound_effect_start_input::fire_and_forget(explosion_location).set_listener(subject_if_any)
	);

	auto& cosmos = step.get_cosmos();

	const auto delta = cosmos.get_fixed_delta();
	const auto now = cosmos.get_timestamp();

	const auto effective_radius_sq = effective_radius*effective_radius;
	const auto subject = cosmos[subject_if_any];
	const auto subject_alive = subject.alive();

	messages::visibility_information_request request;
	request.eye_transform = explosion_location;
	request.filter = filters::line_of_sight_query();
	request.square_side = effective_radius * 2;
	request.subject = subject_if_any;

	const auto response = visibility_system(DEBUG_LOGIC_STEP_LINES).respond_to_visibility_information_requests(
		cosmos,
		{},
		{ request }
	);

	const auto& physics = cosmos.get_solvable_inferred().physics;

	std::unordered_set<unversioned_entity_id> affected_entities_of_bodies;

	for (auto i = 0u; i < response.vis[0].get_num_triangles(); ++i) {
		auto damaging_triangle = response.vis[0].get_world_triangle(i, request.eye_transform.pos);
		damaging_triangle[1] += (damaging_triangle[1] - damaging_triangle[0]).set_length(5);
		damaging_triangle[2] += (damaging_triangle[2] - damaging_triangle[0]).set_length(5);

		physics.for_each_intersection_with_triangle(
			cosmos.get_si(),
			damaging_triangle,
			filters::dynamic_object(),
			[&](
				const b2Fixture* const fix,
				const vec2 point_a,
				const vec2 point_b
			) {
				const auto body_entity_id = get_body_entity_that_owns(fix);
				const bool is_self = 
					subject_alive
					&& (
						body_entity_id == subject.get_id()
						|| cosmos[body_entity_id].get_owning_transfer_capability() == subject.get_id()
					)
				;

				if (is_self) {
					return callback_result::CONTINUE;
				}

				const bool in_range =
					(explosion_location.pos - point_a).length_sq() <= effective_radius_sq
					|| (explosion_location.pos - point_b).length_sq() <= effective_radius_sq
				;

				const bool should_be_affected = in_range;

				if (should_be_affected) {
					const auto it = affected_entities_of_bodies.insert(body_entity_id);
					const bool is_yet_unaffected = it.second;

					if (is_yet_unaffected) {
						const auto body_entity = cosmos[body_entity_id];
						const auto& affected_physics = body_entity.get<components::rigid_body>();
						const auto affected_physics_mass_pos = affected_physics.get_mass_position();

						auto impact = (point_b - explosion_location.pos).set_length(impact_force);
						const auto center_offset = (point_b - affected_physics_mass_pos) * 0.8f;

						messages::damage_message damage_msg;
						damage_msg.type = type;
						damage_msg.inflictor = subject;
						damage_msg.subject = body_entity;
						damage_msg.amount = damage;
						damage_msg.point_of_impact = point_b;

						switch (type) {
							case adverse_element_type::FORCE: {
								damage_msg.request_shake_for_ms = 400.f;
								damage_msg.impact_velocity = impact;

								affected_physics.apply_impulse(
									impact, center_offset
								);

								if (DEBUG_DRAWING.draw_explosion_forces) {
									DEBUG_PERSISTENT_LINES.emplace_back(cyan,
										affected_physics_mass_pos + center_offset,
										affected_physics_mass_pos + center_offset + impact
									);
								}

								// LOG("Impact %x dealt to: %x. Resultant angular: %x", impact, body_entity.get_name(), affected_physics.get_degree_velocity());
							}
							break;

							case adverse_element_type::PED: {

							}
							break;

							case adverse_element_type::INTERFERENCE: {
								damage_msg.request_shake_for_ms = 800.f;
								damage_msg.impact_velocity = impact * 300.f;
							}
							break;

							default: {
								ensure(false && "Unknown adverse element type");
							}
							break;
						}

						step.post_message(damage_msg);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius / 2;
		ring.outer_radius_end_value = effective_radius;

		ring.inner_radius_start_value = 0.f;
		ring.inner_radius_end_value = effective_radius;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = 0.20f;

		ring.color = inner_ring_color;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.post_message(ring);
	}

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius;
		ring.outer_radius_end_value = effective_radius / 2;

		ring.inner_radius_start_value = effective_radius / 1.5f;
		ring.inner_radius_end_value = effective_radius / 2;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = 0.20f;

		ring.color = outer_ring_color;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.post_message(ring);
	}
}