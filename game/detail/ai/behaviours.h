#pragma once
#include "game/resources/behaviour_tree.h"

namespace behaviours {
	typedef resources::behaviour_tree tree;
	typedef tree::node behaviour;

	struct immediate_evasion : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct usable_item_marker : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct navigate_to_useful_item : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct pursue_lost_target : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct target_closest_enemy : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct pull_trigger : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct minimize_recoil_through_movement : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};
}