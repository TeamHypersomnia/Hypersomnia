#include "behaviours.h"
#include "game/components/attitude_component.h"
#include "game/components/visibility_component.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/pathfinding_component.h"
#include "game/components/rotation_copying_component.h"
#include "entity_system/entity.h"

#include "game/detail/entity_scripts.h"
#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"

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

		movement.set_flags_from_closest_direction(resultant_evasion);
	}


	tree::goal_availability target_closest_enemy::goal_resolution(tree::state_of_traversal& t) const {
		return tree::goal_availability::SHOULD_EXECUTE;
	}
	// a*x^2 + b*x + c = 0
	float first_positive_solution_of_quadratic_equation(float a, float b, float c) {
		float discriminant = b*b - 4.0f*a*c;
		if (discriminant < 0.0f)
			return -1.0f; // Indicate there is no solution                                                                      
		float s = std::sqrt(discriminant);
		float x1 = (-b - s) / (2.0f*a);
		if (x1 > 0.0f)
			return x1;
		float x2 = (-b + s) / (2.0f*a);
		if (x2 > 0.0f)
			return x2;
		return -1.0f; // Indicate there is no positive solution                                                               
	}

	vec2 direct_solution(vec2 target_position, vec2 target_velocity, float projectile_speed) {
		float a = target_velocity.dot(target_velocity) - projectile_speed * projectile_speed;
		float b = 2.0f * target_position.dot(target_velocity);
		float c = target_position.dot(target_position);

		float t = first_positive_solution_of_quadratic_equation(a, b, c);
		if (t <= 0.0f)
			return vec2(); // Indicate we failed to find a solution

		return target_position + t * target_velocity;
	}

	void target_closest_enemy::execute_leaf_goal_callback(tree::execution_occurence occ, tree::state_of_traversal& t) const {
		if (occ == tree::execution_occurence::LAST)
			return;
		
		auto subject = t.instance.user_input;
		auto pos = position(subject);
		auto& visibility = subject->get<components::visibility>();
		auto& los = visibility.line_of_sight_layers[components::visibility::LINE_OF_SIGHT];
		auto& attitude = subject->get<components::attitude>();

		augs::entity_id closest_hostile;

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

		if (subject.has(sub_entity_name::CHARACTER_CROSSHAIR)) {
			auto crosshair = subject[sub_entity_name::CHARACTER_CROSSHAIR];
			auto& crosshair_offset = crosshair->get<components::crosshair>().base_offset;

			float vel1 = assess_projectile_velocity_of_weapon(subject[slot_function::PRIMARY_HAND].try_get_item());
			float vel2 = assess_projectile_velocity_of_weapon(subject[slot_function::SECONDARY_HAND].try_get_item());
			float vel = std::max(vel1, vel2);

			if (vel > 1.0 && closest_hostile.alive()) {
				vec2 leaded;

				if (velocity(closest_hostile).length_sq() > 1)
					leaded = direct_solution(position(closest_hostile), velocity(closest_hostile), vel);
				else
					leaded = position(closest_hostile);

				crosshair_offset = leaded - position(subject);;
			}
			else if (is_physical(subject)) {
				crosshair_offset = velocity(subject).length() > 3.0 ? velocity(subject) : vec2(10, 0);
			}
			else
				crosshair_offset = vec2(10, 0);
		}
	}

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
			if(o == tree::execution_occurence::LAST)
				w->get<components::gun>().trigger_pressed = false;
			else
				w->get<components::gun>().trigger_pressed = true;
		}
	}

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

	tree::goal_availability navigate_to_last_seen_position_of_target::goal_resolution(tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& attitude = subject->get<components::attitude>();
		auto currently_attacked_visible_entity = attitude.currently_attacked_visible_entity;

		if (currently_attacked_visible_entity.dead() && attitude.is_alert && !attitude.last_seen_target_position_inspected) {
			return tree::goal_availability::SHOULD_EXECUTE;
		}

		return tree::goal_availability::ALREADY_ACHIEVED;
	}

	void navigate_to_last_seen_position_of_target::execute_leaf_goal_callback(tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& attitude = subject->get<components::attitude>();
		auto& movement = subject->get<components::movement>();
		auto& pathfinding = subject->get<components::pathfinding>();

		if (o == tree::execution_occurence::FIRST) {
			pathfinding.start_pathfinding(attitude.last_seen_target_position);
		}
		else if (o == tree::execution_occurence::LAST) {
			movement.reset_movement_flags();
			pathfinding.stop_and_clear_pathfinding();
		}
		else {
			if (pathfinding.has_pathfinding_finished()) {
				attitude.last_seen_target_position_inspected = true;
			}
			else
				movement.set_flags_from_closest_direction(pathfinding.get_current_navigation_point() - position(subject));
		}
	}

	tree::goal_availability explore_in_search_for_last_seen_target::goal_resolution(tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& attitude = subject->get<components::attitude>();
		auto currently_attacked_visible_entity = attitude.currently_attacked_visible_entity;

		if (currently_attacked_visible_entity.dead() && attitude.is_alert && attitude.last_seen_target_position_inspected) {
			return tree::goal_availability::SHOULD_EXECUTE;
		}

		return tree::goal_availability::ALREADY_ACHIEVED;
	}

	void explore_in_search_for_last_seen_target::execute_leaf_goal_callback(tree::execution_occurence o, tree::state_of_traversal& t) const {
		auto subject = t.instance.user_input;
		auto& attitude = subject->get<components::attitude>();
		auto& movement = subject->get<components::movement>();
		auto& pathfinding = subject->get<components::pathfinding>();

		if (o == tree::execution_occurence::FIRST) {
			pathfinding.start_exploring();
			pathfinding.custom_exploration_hint.enabled = true;
			pathfinding.custom_exploration_hint.origin = attitude.last_seen_target_position;
			pathfinding.custom_exploration_hint.target = attitude.last_seen_target_position + attitude.last_seen_target_velocity;
		}
		else if (o == tree::execution_occurence::LAST) {
			movement.reset_movement_flags();
			pathfinding.stop_and_clear_pathfinding();
		}
		else {
			if (pathfinding.has_exploring_finished()) {
				ensure(0);
			}
			else
				movement.set_flags_from_closest_direction(pathfinding.get_current_navigation_point() - position(subject));
		}
	}
	
}