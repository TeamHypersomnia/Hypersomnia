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

	struct retreat_due_to_low_capabilities : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct head_back_due_to_deficient_capabilities : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct pursue_enemy : behaviour {
		tree::goal_availability goal_resolution(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::state_of_traversal&) const final;
	};

	struct stand_still_with_cyclic_twists {

	};
}