#include "game/detail/ai/behaviours.h"
#include "game/transcendental/entity_id.h"

#include "game/components/attitude_component.h"
#include "game/components/crosshair_component.h"
#include "game/messages/visibility_information.h"
#include "game/components/gun_component.h"
#include "game/detail/entity_scripts.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

namespace behaviours {

	tree::goal_availability target_closest_enemy::goal_resolution(tree::state_of_traversal&) const {
		return tree::goal_availability::SHOULD_EXECUTE;
	}

	void target_closest_enemy::execute_leaf_goal_callback(tree::execution_occurence occ, tree::state_of_traversal& t) const {
		if (occ == tree::execution_occurence::LAST)
			return;

		auto& cosmos = t.step.get_cosmos();
		auto subject = t.get_subject();
		const auto subject_transform = subject.get_logic_transform();
		const auto pos = subject_transform.pos;
		/* auto& los = t.step.transient.calculated_line_of_sight.at(subject); */
		auto& attitude = subject.get<components::attitude>();

		entity_id closest_hostile_raw;

		float min_distance = std::numeric_limits<float>::max();

		(void)min_distance;
		/* for (auto s_raw : los.visible_sentiences) { */
		/* 	auto s = cosmos[s_raw]; */
		/* 	const auto calculated_attitude = calc_attitude(s, subject); */

		/* 	if (is_hostile(calculated_attitude)) { */
		/* 		auto dist = (s.get_logic_transform().pos - pos).length_sq(); */

		/* 		if (dist < min_distance) { */
		/* 			closest_hostile_raw = s; */
		/* 			min_distance = dist; */
		/* 		} */
		/* 	} */
		/* } */

		auto closest_hostile = cosmos[closest_hostile_raw];

		attitude.currently_attacked_visible_entity = closest_hostile;

		const auto closest_hostile_transform = closest_hostile.get_logic_transform();
		const auto closest_hostile_velocity = closest_hostile.get_effective_velocity();

		if (closest_hostile.alive()) {
			attitude.is_alert = true;
			attitude.last_seen_target_position_inspected = false;
			attitude.last_seen_target_position = closest_hostile_transform.pos;
			attitude.last_seen_target_velocity = closest_hostile_velocity;
		}

		if (const auto crosshair = subject.find_crosshair()) {
			auto& crosshair_offset = crosshair->base_offset;

			const auto vel = 
				std::max(
					assess_projectile_velocity_of_weapon(subject.get_if_any_item_in_hand_no(0)),
					assess_projectile_velocity_of_weapon(subject.get_if_any_item_in_hand_no(1))
				)
			;

			if (vel > 1.0 && closest_hostile.alive()) {
				vec2 leaded;

				if (closest_hostile_velocity.length_sq() > 1) {
					leaded = closest_hostile_transform.pos + closest_hostile_velocity * (closest_hostile_transform.pos - pos).length_sq() / vel;// direct_solution(position(closest_hostile), velocity(closest_hostile), vel);
				}
				else {
					leaded = closest_hostile_transform.pos;
				}

				crosshair_offset = leaded - pos;
			}
			else if (subject.has<components::rigid_body>()) {
				const auto subject_vel = subject.get_effective_velocity();

				crosshair_offset = subject_vel.length() > 3.0 ? subject_vel : vec2(10, 0);
			}
			else {
				crosshair_offset = vec2(10, 0);
			}
		}
	}
}