#include "game/detail/ai/behaviours.h"
#include "game/cosmos/entity_id.h"

#include "game/components/attitude_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/detail/entity_scripts.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"

namespace behaviours {
	tree::goal_availability pull_trigger::goal_resolution(tree::state_of_traversal& t) const {
		const auto& cosm = t.step.get_cosmos();
		const auto subject = cosm[t.subject];
		const auto& attitude = subject.get<components::attitude>();
		const auto currently_attacked_visible_entity = t.step.get_cosmos()[attitude.currently_attacked_visible_entity];

		if (currently_attacked_visible_entity.alive() && subject.get_wielded_guns().size() > 0) {
			//if (crosshair_offset.degrees_between(orientation(subject)) < attitude.maximum_divergence_angle_before_shooting) {
			return tree::goal_availability::SHOULD_EXECUTE;
			//}
		}

		return tree::goal_availability::CANT_EXECUTE;
	}

	void pull_trigger::execute_leaf_goal_callback(const tree::execution_occurence o, tree::state_of_traversal& t) const {
		(void)o;
		(void)t;
#if TODO_AI
		auto& cosm = t.step.get_cosmos();
		const auto subject = cosm[t.subject];
		const auto wielded = subject.get_wielded_guns();

		for (const auto w_id : wielded) {
			const auto w = cosm[w_id];

			if (o == tree::execution_occurence::LAST) {
				w.get<components::gun>().is_trigger_pressed = false;
			}
			else {
				w.get<components::gun>().is_trigger_pressed = true;
			}
		}
#endif
	}
}