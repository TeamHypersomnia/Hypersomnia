#include "behaviour_tree.h"
#include "ensure.h"

namespace resources {
	behaviour_tree::node& behaviour_tree::get_node_by_id(int i) const {
		return *node_pointers[i];
	}

	void behaviour_tree::state_of_tree_instance::reset() {
		previously_executed_leaf_id = -1;
		user_input = user_callback_input();
	}
	
	void behaviour_tree::dfs(node& p, std::function<void(node&)> f) {
		f(p);

		for (auto& c : p.children)
			dfs(*c, f);
	}

	void behaviour_tree::dfs_all(std::function<void(node&)> f) {
		dfs(root, f);
	}

	void behaviour_tree::build_tree() {
		int id_counter = 0;

		node_pointers.clear();
		
		dfs_all([&id_counter, this](node& n){
			node_pointers.push_back(&n);
			n.id_in_tree = id_counter++;
		});
	}

	void behaviour_tree::evaluate_instance_of_tree(state_of_tree_instance& inst) const {
		state_of_traversal traversal = { inst, *this };
		root.evaluate_node(traversal);
	}

	behaviour_tree::determined_goal behaviour_tree::node::determine_goal_availability(behaviour_tree::state_of_traversal&) const {
		return{ goal_availability::SHOULD_EXECUTE, nullptr };
	}

	void behaviour_tree::node::execute_leaf_goal_callback(execution_occurence, goal_ptr, state_of_traversal&) const {
		ensure(false);
	}

	behaviour_tree::goal_availability behaviour_tree::node::evaluate_node(state_of_traversal& traversal) const {
		ensure(id_in_tree != -1);

		auto goal = determine_goal_availability(traversal);

		if (goal.availability == goal_availability::SHOULD_EXECUTE) {
			bool traverse_further_to_determine_status = !children.empty();

			if (traverse_further_to_determine_status) {
				if (mode == type::SEQUENCER) {
					for (auto& child : children) {
						auto availability = child->evaluate_node(traversal);

						if (availability == goal_availability::ALREADY_ACHIEVED)
							continue;
						else if (availability == goal_availability::CANT_EXECUTE) 
							return goal_availability::CANT_EXECUTE;
						else if (availability == goal_availability::SHOULD_EXECUTE) 
							return goal_availability::SHOULD_EXECUTE;
					}

					return goal_availability::ALREADY_ACHIEVED;
				}
				else if (mode == type::SELECTOR) {
					for (auto& child : children) {
						auto availability = child->evaluate_node(traversal);

						if (availability == goal_availability::ALREADY_ACHIEVED) 
							continue;
						else if (availability == goal_availability::CANT_EXECUTE) 
							continue;
						else if (availability == goal_availability::SHOULD_EXECUTE) 
							return goal_availability::SHOULD_EXECUTE;
					}

					return goal_availability::CANT_EXECUTE;
				}
				else
					ensure(false);
			} 
			else {
				int previously_executed_id = traversal.instance.previously_executed_leaf_id;

				bool is_repeated_execution = previously_executed_id == id_in_tree;
				bool notify_previous_and_perform_first_occurence = !is_repeated_execution;

				auto& newly_executed = traversal.original_tree.get_node_by_id(id_in_tree);

				if (notify_previous_and_perform_first_occurence) {
					auto& previously_executed = traversal.original_tree.get_node_by_id(previously_executed_id);
					previously_executed.execute_leaf_goal_callback(execution_occurence::LAST, nullptr, traversal);
					newly_executed.execute_leaf_goal_callback(execution_occurence::FIRST, std::move(goal.data), traversal);
				}
				else {
					newly_executed.execute_leaf_goal_callback(execution_occurence::REPEATED, std::move(goal.data), traversal);
				}
				
				traversal.instance.previously_executed_leaf_id = id_in_tree;
			}
		}
	}
}

