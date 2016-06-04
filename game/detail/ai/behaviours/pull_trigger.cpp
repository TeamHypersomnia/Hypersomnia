#include "game/detail/ai/behaviours.h"
#include "entity_system/entity.h"

#include "game/components/attitude_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/position_scripts.h"

namespace behaviours {
	tree::goal_availability pull_trigger::goal_resolution(tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& attitude = subject->get<components::attitude>();
		auto currently_attacked_visible_entity = attitude.currently_attacked_visible_entity;
		auto crosshair = subject[sub_entity_name::CHARACTER_CROSSHAIR];
		auto& crosshair_offset = crosshair->get<components::crosshair>().base_offset;

		if (currently_attacked_visible_entity.alive() && guns_wielded(subject).size() > 0) {
			//if (crosshair_offset.degrees_between(orientation(subject)) < attitude.maximum_divergence_angle_before_shooting) {
			return tree::goal_availability::SHOULD_EXECUTE;
			//}
		}

		return tree::goal_availability::CANT_EXECUTE;
	}

	void pull_trigger::execute_leaf_goal_callback(tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto wielded = guns_wielded(subject);

		for (auto& w : wielded) {
			if (o == tree::execution_occurence::LAST)
				w->get<components::gun>().trigger_pressed = false;
			else
				w->get<components::gun>().trigger_pressed = true;
		}
	}
}