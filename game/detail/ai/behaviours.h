#pragma once
#include "game/resources/behaviour_tree.h"


namespace behaviours {
	typedef resources::behaviour_tree tree;
	typedef tree::node behaviour;

	struct immediate_evasion : behaviour {
		float minimum_distance = 0.f;

		tree::determined_goal determine_goal_availability(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::goal_ptr, tree::state_of_traversal&) const final;
	};

	struct usable_item_marker : behaviour {
		tree::determined_goal determine_goal_availability(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::goal_ptr, tree::state_of_traversal&) const final;
	};

	struct navigate_to_useful_item : behaviour {
		tree::determined_goal determine_goal_availability(tree::state_of_traversal&) const final;
		void execute_leaf_goal_callback(tree::execution_occurence, tree::goal_ptr, tree::state_of_traversal&) const final;
	};
}