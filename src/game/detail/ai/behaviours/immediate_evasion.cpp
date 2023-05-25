#include "game/detail/ai/behaviours.h"
#include "game/cosmos/entity_id.h"

#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/movement_component.h"
#include "game/messages/visibility_information.h"
#include "game/detail/entity_scripts.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"

namespace behaviours {
	tree::goal_availability immediate_evasion::goal_resolution(tree::state_of_traversal& t) const {
		//const auto& los = t.step.transient.calculated_line_of_sight.at(subject);
		const auto& cosm = t.step.get_cosmos();
		const auto subject = cosm[t.subject];

		immediate_evasion_goal goal;

		float total_danger = 0.f;

		(void)cosm;

		/* for (auto s : los.visible_dangers) { */
		/* 	const auto danger = assess_danger(subject, cosm[s]); */
		/* 	ensure(danger.amount > 0); */

		/* 	total_danger += danger.amount; */
		/* 	goal.dangers.push_back(danger); */
		/* } */

		if (total_danger < subject.get<invariants::sentience>().minimum_danger_amount_to_evade) {
			return tree::goal_availability::ALREADY_ACHIEVED;
		}
		else {
			t.set_goal(goal);
			return tree::goal_availability::SHOULD_EXECUTE;
		}
	}

	void immediate_evasion::execute_leaf_goal_callback(const tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto& cosm = t.step.get_cosmos();
		const auto subject = cosm[t.subject];

		auto& movement = subject.get<components::movement>();

		if (o == tree::execution_occurence::LAST) {
			movement.reset_movement_flags();
			return;
		}

		const auto& goal = t.get_goal<immediate_evasion_goal>();

		vec2 resultant_evasion;

		for (size_t i = 0; i < goal.dangers.size(); ++i) {
			resultant_evasion += goal.dangers[i].recommended_evasion * goal.dangers[i].amount;
		}

		movement.flags.set_from_closest_direction(resultant_evasion);
	}
}