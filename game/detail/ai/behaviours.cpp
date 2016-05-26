#include "behaviours.h"
#include "game/components/attitude_component.h"
#include "game/components/visibility_component.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"
#include "entity_system/entity.h"

#include "game/detail/entity_scripts.h"

namespace behaviours {
	tree::goal_availability immediate_evasion::goal_resolution(tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& visibility = subject->get<components::visibility>();
		auto& los = visibility.line_of_sight_layers[components::visibility::LINE_OF_SIGHT];

		immediate_evasion_goal goal;

		float total_danger = 0.f;

		for (auto& s : los.visible_dangers) {
			auto danger = assess_danger(subject, s);
			ensure(danger.amount > 0);

			total_danger += danger.amount;
			goal.dangers.push_back(danger);
		}

		if (total_danger < subject->get<components::sentience>().minimum_danger_amount_to_evade) {
			return tree::goal_availability::ALREADY_ACHIEVED;
		}
		else {
			t.set_goal(goal);
			return tree::goal_availability::SHOULD_EXECUTE;
		}
	}

	void immediate_evasion::execute_leaf_goal_callback(tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& movement = subject->get<components::movement>();

		if (o == tree::execution_occurence::LAST) {
			movement.reset_movement_flags();
			return;
		}

		auto& goal = t.get_goal<immediate_evasion_goal>();

		vec2 resultant_evasion;

		for (size_t i = 0; i < goal.dangers.size(); ++i) {
			resultant_evasion += goal.dangers[i].recommended_evasion * goal.dangers[i].amount;
		}

		movement.set_flags_from_target_direction(resultant_evasion);
	}


	tree::goal_availability target_closest_enemy::goal_resolution(tree::state_of_traversal& t) const {
		return tree::goal_availability::SHOULD_EXECUTE;
	}

	void target_closest_enemy::execute_leaf_goal_callback(tree::execution_occurence occ, tree::state_of_traversal& t) const {
		if (occ == tree::execution_occurence::LAST)
			return;
		
		auto subject = t.instance.user_input;
		auto& visibility = subject->get<components::visibility>();
		auto& los = visibility.line_of_sight_layers[components::visibility::LINE_OF_SIGHT];

		augs::entity_id closest_hostile;

		float min_distance = std::numeric_limits<float>::max();

		for (auto& s : los.visible_sentiences) {
			auto att = calculate_attitude(s, subject);

			if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
				if ((s->get<components::transform>().pos - subject->get<components::transform>().pos).length_sq() < min_distance) {
					closest_hostile = s;
				}
			}
		}

		subject->get<components::attitude>().chosen_target = closest_hostile;
	}
}