#include "game/detail/ai/behaviours.h"
#include "game/entity_id.h"

#include "game/components/attitude_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/visibility_component.h"
#include "game/components/gun_component.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/position_scripts.h"

#include "game/detail/inventory_utils.h"

namespace behaviours {

	tree::goal_availability target_closest_enemy::goal_resolution(tree::state_of_traversal& t) const {
		return tree::goal_availability::SHOULD_EXECUTE;
	}

	void target_closest_enemy::execute_leaf_goal_callback(tree::execution_occurence occ, tree::state_of_traversal& t) const {
		if (occ == tree::execution_occurence::LAST)
			return;

		auto subject = t.instance.user_input;
		auto pos = position(subject);
		auto& visibility = subject.get<components::visibility>();
		auto& los = visibility.line_of_sight_layers[components::visibility::LINE_OF_SIGHT];
		auto& attitude = subject.get<components::attitude>();

		entity_id closest_hostile;

		float min_distance = std::numeric_limits<float>::max();

		for (auto& s : los.visible_sentiences) {
			auto att = calculate_attitude(s, subject);

			if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
				auto dist = distance_sq(s, subject);

				if (dist < min_distance) {
					closest_hostile = s;
					min_distance = dist;
				}
			}
		}

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

			float vel1 = assess_projectile_velocity_of_weapon(subject[slot_function::PRIMARY_HAND].try_get_item());
			float vel2 = assess_projectile_velocity_of_weapon(subject[slot_function::SECONDARY_HAND].try_get_item());
			float vel = std::max(vel1, vel2);

			if (vel > 1.0 && closest_hostile.alive()) {
				vec2 leaded;

				if (velocity(closest_hostile).length_sq() > 1)
					leaded = position(closest_hostile) + velocity(closest_hostile) * distance(closest_hostile, subject) / vel;// direct_solution(position(closest_hostile), velocity(closest_hostile), vel);
				else
					leaded = position(closest_hostile);

				crosshair_offset = leaded - position(subject);
			}
			else if (is_physical(subject)) {
				crosshair_offset = velocity(subject).length() > 3.0 ? velocity(subject) : vec2(10, 0);
			}
			else
				crosshair_offset = vec2(10, 0);
		}
	}
}