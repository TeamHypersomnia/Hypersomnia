#include "game/detail/ai/behaviours.h"
#include "game/transcendental/entity_id.h"

#include "game/components/attitude_component.h"
#include "game/components/crosshair_component.h"
#include "game/messages/visibility_information.h"
#include "game/components/gun_component.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/position_scripts.h"

#include "game/detail/inventory_utils.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/transcendental/data_living_one_step.h"

namespace behaviours {

	tree::goal_availability target_closest_enemy::goal_resolution(tree::state_of_traversal& t) const {
		return tree::goal_availability::SHOULD_EXECUTE;
	}

	void target_closest_enemy::execute_leaf_goal_callback(tree::execution_occurence occ, tree::state_of_traversal& t) const {
		if (occ == tree::execution_occurence::LAST)
			return;

		auto& cosmos = t.step.cosm;
		auto subject = t.subject;
		auto pos = position(subject);
		auto& los = t.step.transient.calculated_line_of_sight.at(subject);
		auto& attitude = subject.get<components::attitude>();

		entity_id closest_hostile_raw;

		float min_distance = std::numeric_limits<float>::max();

		for (auto s_raw : los.visible_sentiences) {
			auto s = cosmos[s_raw];
			const auto calculated_attitude = calculate_attitude(s, subject);

			if (is_hostile(calculated_attitude)) {
				auto dist = distance_sq(s, subject);

				if (dist < min_distance) {
					closest_hostile_raw = s;
					min_distance = dist;
				}
			}
		}

		auto closest_hostile = cosmos[closest_hostile_raw];

		attitude.currently_attacked_visible_entity = closest_hostile;

		if (closest_hostile.alive()) {
			attitude.is_alert = true;
			attitude.last_seen_target_position_inspected = false;
			attitude.last_seen_target_position = position(closest_hostile);
			attitude.last_seen_target_velocity = velocity(closest_hostile);
		}

		auto crosshair = subject[sub_entity_name::CHARACTER_CROSSHAIR];

		if (crosshair.alive()) {
			auto& crosshair_offset = crosshair.get<components::crosshair>().base_offset;

			float vel1 = assess_projectile_velocity_of_weapon(subject[slot_function::PRIMARY_HAND].get_item_if_any());
			float vel2 = assess_projectile_velocity_of_weapon(subject[slot_function::SECONDARY_HAND].get_item_if_any());
			float vel = std::max(vel1, vel2);

			if (vel > 1.0 && closest_hostile.alive()) {
				vec2 leaded;

				if (velocity(closest_hostile).length_sq() > 1) {
					leaded = position(closest_hostile) + velocity(closest_hostile) * distance(closest_hostile, subject) / vel;// direct_solution(position(closest_hostile), velocity(closest_hostile), vel);
				}
				else {
					leaded = position(closest_hostile);
				}

				crosshair_offset = leaded - position(subject);
			}
			else if (is_entity_physical(subject)) {
				crosshair_offset = velocity(subject).length() > 3.0 ? velocity(subject) : vec2(10, 0);
			}
			else {
				crosshair_offset = vec2(10, 0);
			}
		}
	}
}