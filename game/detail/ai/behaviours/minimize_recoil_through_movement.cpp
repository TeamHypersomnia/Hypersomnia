#include "game/detail/ai/behaviours.h"
#include "game/entity_id.h"

#include "game/components/attitude_component.h"
#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/position_scripts.h"

namespace behaviours {
	tree::goal_availability minimize_recoil_through_movement::goal_resolution(tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto crosshair = subject[sub_entity_name::CHARACTER_CROSSHAIR];
		auto& attitude = subject->get<components::attitude>();
		auto currently_attacked_visible_entity = attitude.currently_attacked_visible_entity;

		if (currently_attacked_visible_entity.alive() && crosshair.alive()) {
			auto recoil = crosshair[sub_entity_name::CROSSHAIR_RECOIL_BODY];
			auto& c = crosshair->get<components::crosshair>();

			minimize_recoil_through_movement_goal goal;

			goal.movement_direction = (c.base_offset - orientation(subject).set_length(c.base_offset.length()));
			t.set_goal(goal);
			return tree::goal_availability::SHOULD_EXECUTE;
		}

		return tree::goal_availability::ALREADY_ACHIEVED;
	}

	void minimize_recoil_through_movement::execute_leaf_goal_callback(tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& movement = subject->get<components::movement>();

		if (o == tree::execution_occurence::LAST)
			movement.reset_movement_flags();
		else
			movement.set_flags_from_closest_direction(t.get_goal<minimize_recoil_through_movement_goal>().movement_direction);
	}
}