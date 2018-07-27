#include "augs/ensure.h"
#include "game/assets/behaviour_tree.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

behaviour_tree::state_of_traversal::state_of_traversal(
	const logic_step step,
	const entity_id subject,
	state_of_behaviour_tree_instance& instance,
	const behaviour_tree& original_tree
) :
	step(step),
	subject(subject),
	instance(instance),
	original_tree(original_tree)
{
	std::fill(goals_set.begin(), goals_set.end(), false);
}

const behaviour_tree::node& behaviour_tree::get_node_by_id(const int i) const {
	return *tree_nodes[i];
}

void behaviour_tree::build_tree() {
	int id_counter = 0;

	tree_nodes.clear();

	call_on_node_recursively(root, [&id_counter, this](node& n) {
		tree_nodes.push_back(&n);
		n.this_id_in_tree = id_counter++;
	});
}

void behaviour_tree::evaluate_instance_of_tree(
	const logic_step step,
	const entity_id id,
	state_of_behaviour_tree_instance& inst
) const {
	state_of_traversal traversal(step, id, inst, *this);

	const auto tree_evaluation_result = root.evaluate_node(traversal);

	const auto previous_id = inst.previously_executed_leaf_id;
	const bool previously_executed_node_exists = previous_id != -1;
	const bool terminate_previously_running_node = previously_executed_node_exists && tree_evaluation_result != goal_availability::SHOULD_EXECUTE;

	if (terminate_previously_running_node) {
		const auto& previously_executed_node = traversal.original_tree.get_node_by_id(previous_id);
		previously_executed_node.execute_leaf_goal_callback(execution_occurence::LAST, traversal);
	}
}

behaviour_tree::node* behaviour_tree::node::create_branches() {
	return this;
}

behaviour_tree::goal_availability behaviour_tree::node::goal_resolution(behaviour_tree::state_of_traversal&) const {
	return goal_availability::SHOULD_EXECUTE;
}

void behaviour_tree::node::execute_leaf_goal_callback(const execution_occurence, state_of_traversal&) const {
	// ensure(false && "Undefined action callback! Perhaps root has no children?");
}

behaviour_tree::goal_availability behaviour_tree::node::evaluate_node(state_of_traversal& traversal) const {
	ensure(this_id_in_tree != -1);

	const auto this_availability = goal_resolution(traversal);

	if (this_availability == goal_availability::SHOULD_EXECUTE) {
		const bool is_leaf = children.empty();

		if (is_leaf) {
			const int previously_executed_id = traversal.instance.previously_executed_leaf_id;
			const bool is_execution_repeated = previously_executed_id == this_id_in_tree;

			if (is_execution_repeated) {
				execute_leaf_goal_callback(execution_occurence::REPEATED, traversal);
			}
			else {
				const bool previous_exists_and_must_be_terminated = previously_executed_id != -1;

				if (previous_exists_and_must_be_terminated) {
					auto& previously_executed_node = traversal.original_tree.get_node_by_id(previously_executed_id);
					previously_executed_node.execute_leaf_goal_callback(execution_occurence::LAST, traversal);
				}

				execute_leaf_goal_callback(execution_occurence::FIRST, traversal);
			}

			traversal.instance.previously_executed_leaf_id = this_id_in_tree;
		}
		else {
			if (mode == type::SEQUENCER) {
				for (const auto& child : children) {
					const auto child_availability = child.evaluate_node(traversal);

					if (child_availability == goal_availability::ALREADY_ACHIEVED) {
						continue;
					}
					else if (child_availability == goal_availability::CANT_EXECUTE) {
						return goal_availability::CANT_EXECUTE;
					}
					else if (child_availability == goal_availability::SHOULD_EXECUTE) {
						return goal_availability::SHOULD_EXECUTE;
					}
				}

				return goal_availability::ALREADY_ACHIEVED;
			}
			else if (mode == type::SELECTOR) {
				for (const auto& child : children) {
					const auto child_availability = child.evaluate_node(traversal);

					if (child_availability == goal_availability::ALREADY_ACHIEVED) {
						continue;
					}
					else if (child_availability == goal_availability::CANT_EXECUTE) {
						continue;
					}
					else if (child_availability == goal_availability::SHOULD_EXECUTE) {
						return goal_availability::SHOULD_EXECUTE;
					}
				}

				return goal_availability::CANT_EXECUTE;
			}
			else {
				ensure(false);
			}
		}
	}

	return this_availability;
}

