#include "behaviour_tree.h"
#include "augs/ensure.h"
#include "game/transcendental/entity_handle.h"

namespace resources {
	behaviour_tree::state_of_traversal::state_of_traversal(fixed_step& step, entity_handle subject, state_of_tree_instance& in, const behaviour_tree& bt)
		: step(step), instance(in), original_tree(bt), subject(subject) {
		std::fill(goals_set.begin(), goals_set.end(), false);
	}

	const behaviour_tree::node& behaviour_tree::get_node_by_id(int i) const {
		return *node_pointers[i];
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

	void behaviour_tree::evaluate_instance_of_tree(fixed_step& step, entity_handle handle, state_of_tree_instance& inst) const {
		state_of_traversal traversal(step, handle, inst, *this);
		auto result = root.evaluate_node(traversal);

		int previous_id = inst.previously_executed_leaf_id;
		bool terminate_maybe_previously_running_node = (result != goal_availability::SHOULD_EXECUTE) && previous_id != -1;

		if (terminate_maybe_previously_running_node) {
			auto& previously_executed = traversal.original_tree.get_node_by_id(previous_id);
			previously_executed.execute_leaf_goal_callback(execution_occurence::LAST, traversal);
		}
	}

	behaviour_tree::node* behaviour_tree::node::branch() {
		return this;
	}

	behaviour_tree::goal_availability behaviour_tree::node::goal_resolution(behaviour_tree::state_of_traversal&) const {
		return goal_availability::SHOULD_EXECUTE;
	}

	void behaviour_tree::node::execute_leaf_goal_callback(execution_occurence, state_of_traversal&) const {
		// ensure(false && "Undefined action callback! Perhaps root has no children?");
	}

	behaviour_tree::goal_availability behaviour_tree::node::evaluate_node(state_of_traversal& traversal) const {
		ensure(id_in_tree != -1);

		auto availability = goal_resolution(traversal);

		if (availability == goal_availability::SHOULD_EXECUTE) {
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

				bool is_execution_repeated = previously_executed_id == id_in_tree;
				bool notify_previous_and_perform_first_occurence = !is_execution_repeated;

				auto& next_executed = traversal.original_tree.get_node_by_id(id_in_tree);

				if (notify_previous_and_perform_first_occurence) {
					if (previously_executed_id != -1) {
						auto& previously_executed = traversal.original_tree.get_node_by_id(previously_executed_id);
						previously_executed.execute_leaf_goal_callback(execution_occurence::LAST, traversal);
					}

					next_executed.execute_leaf_goal_callback(execution_occurence::FIRST, traversal);
				}
				else {
					next_executed.execute_leaf_goal_callback(execution_occurence::REPEATED, traversal);
				}
				
				traversal.instance.previously_executed_leaf_id = id_in_tree;
			}
		}

		return availability;
	}
}

