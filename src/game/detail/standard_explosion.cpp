#include "augs/misc/randomization.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/standard_explosion.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/messages/exploding_ring_input.h"
#include "game/messages/damage_message.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/enums/filters.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/debug_drawing_settings.h"
#include "game/messages/thunder_input.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/physics/shape_overlapping.hpp"
#include "game/detail/damage_origin.hpp"

static bool triangle_degenerate(const std::array<vec2, 3>& v) {
	constexpr auto eps_triangle_degenerate = 0.5f;

	if (v[0].compare(v[1], eps_triangle_degenerate)) {
		return true;
	}

	if (v[0].compare(v[2], eps_triangle_degenerate)) {
		return true;
	}

	if (v[1].compare(v[2], eps_triangle_degenerate)) {
		return true;
	}

	return false;
}

void standard_explosion_input::instantiate(
	const logic_step step,
	const transformr explosion_location,
	const damage_cause cause
) const {
	const auto subject_if_any = cause.entity;

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

	{
		sound_effect_input effect;
		effect.id = sound_effect; 
		effect.modifier.gain = sound_gain;

		effect.start(
			step,
			sound_effect_start_input::fire_and_forget(explosion_location).set_listener(subject_if_any)
		);
	}

	auto& cosm = step.get_cosmos();

	const auto si = cosm.get_si();
	const auto now = cosm.get_timestamp();

	const auto subject = cosm[subject_if_any];
	const auto subject_alive = subject.alive();

	if (subject_alive) {
		if (const auto sentience = subject.find<components::sentience>()) {
			subject_shake.apply(now, *sentience);
		}
	}

	const auto explosion_pos = explosion_location.pos;

	if (this->type != adverse_element_type::PED) {
		startle_nearby_organisms(cosm, explosion_pos, effective_radius * 1.8f, 60.f, startle_type::IMMEDIATE);
	}

	messages::visibility_information_request request;
	request.eye_transform = explosion_location;
	request.filter = predefined_queries::pathfinding();
	request.square_side = effective_radius * 2;
	request.subject = subject_if_any;

	auto& response = visibility_system(DEBUG_LOGIC_STEP_LINES).calc_visibility(cosm, request);

	if (response.empty()) {
		return;
	}

	const auto& physics = cosm.get_solvable_inferred().physics;

	std::unordered_set<unversioned_entity_id> affected_entities_of_bodies;

	for (auto i = 0u; i < response.get_num_triangles(); ++i) {
		auto damaging_triangle = response.get_world_triangle(i, request.eye_transform.pos);
		damaging_triangle[1] += (damaging_triangle[1] - damaging_triangle[0]).set_length(5);
		damaging_triangle[2] += (damaging_triangle[2] - damaging_triangle[0]).set_length(5);

		if (triangle_degenerate(damaging_triangle)) {
			continue;
		}

		physics.for_each_intersection_with_triangle(
			cosm.get_si(),
			damaging_triangle,
			filters[predefined_filter_type::WALL],
			[&](
				const b2Fixture& fix,
				const vec2 point_a,
				const vec2 point_b
			) {
				(void)point_a;

				const auto victim_id = get_entity_that_owns(fix);
				const auto victim = cosm[victim_id];

				const bool is_self = 
					subject_alive
					&& (
						victim_id == FixtureUserdata(subject.get_id())
						|| victim.get_owning_transfer_capability() == subject.get_id()
					)
				;

				if (is_self) {
					return callback_result::CONTINUE;
				}

				const bool is_explosion_body = victim.has<components::cascade_explosion>();

				if (is_explosion_body) {
					return callback_result::CONTINUE;
				}

				const bool in_range = [&]() {
					b2CircleShape shape;
					shape.m_radius = si.get_meters(effective_radius);

					if (const auto result = shape_overlaps_fixture(&shape, si, explosion_pos, fix)) {
						return true;
					}

					return false;
				}();

				const bool should_be_affected = in_range;

				if (should_be_affected) {
					const auto it = affected_entities_of_bodies.insert(victim_id);
					const bool is_yet_unaffected = it.second;

					if (is_yet_unaffected) {
						messages::damage_message damage_msg;
						damage_msg.type = this->type;
						damage_msg.origin.cause = cause;
						damage_msg.origin.copy_sender_from(subject);
						damage_msg.subject = victim;
						damage_msg.damage = damage;
						damage_msg.impact_velocity = (point_b - explosion_pos).normalize();
						damage_msg.point_of_impact = point_b;

						if (type == adverse_element_type::INTERFERENCE) {
							// TODO: move this calculation after refactoring sentience system to not use messages?
							auto& amount = damage_msg.damage.base;
							amount *= 1 + victim.get_effective_velocity().length() / 400.f;
						}

						step.post_message(damage_msg);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	{
		physics.for_each_intersection_with_circle_meters( 
			si,
			si.get_meters(effective_radius) * 2,
			explosion_location.to<b2Transform>(si),
			filters[predefined_filter_type::CHARACTER],
			[&](
				const b2Fixture& fix,
				const vec2,
				const vec2
			) {
				const auto victim_id = get_entity_that_owns(fix);
				const auto it = affected_entities_of_bodies.insert(victim_id);
				const bool is_yet_unaffected = it.second;

				if (is_yet_unaffected) {
					const auto victim = cosm[victim_id];

					if (const auto sentience = victim.find<components::sentience>()) {
						auto lesser_shake = damage.shake;
						lesser_shake *= 0.4f;
						lesser_shake.apply(now, *sentience);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	// TODO_PERFORMANCE: This code is unnecessary for the server

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius / 2;
		ring.outer_radius_end_value = effective_radius;

		ring.inner_radius_start_value = 0.f;
		ring.inner_radius_end_value = effective_radius;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = ring_duration_seconds;

		ring.color = inner_ring_color;
		ring.center = explosion_pos;
		ring.visibility = response;

		step.post_message(ring);
	}

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius;
		ring.outer_radius_end_value = effective_radius / 2;

		ring.inner_radius_start_value = effective_radius / 1.5f;
		ring.inner_radius_end_value = effective_radius / 2;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = ring_duration_seconds;

		ring.color = outer_ring_color;
		ring.center = explosion_pos;
		ring.visibility = response;

		step.post_message(ring);
	}
}