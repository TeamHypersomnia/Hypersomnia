#include "stdafx.h"
#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

using namespace components;

behaviour_tree::behaviour::behaviour() : current_status(status::INVALID), node_type(type::SEQUENCER) {}

void behaviour_tree::tree::flatten_routine(behaviour* node) {
	/* depth first order */
	auto found_entry = address_map.find(node);
	
	/* count only if it is unique as it is DAG */
	if (found_entry == address_map.end()) {
		address_map[node] = flattened_tree.size();
		flattened_tree.push_back(*node);

		for (auto child : node->children)
			flatten_routine(child);
	}
}

void behaviour_tree::tree::create_flattened_tree(behaviour* root) {
	flattened_tree.clear();
	address_map.clear();

	flatten_routine(root);

	for (auto& node : flattened_tree) 
		for (auto& child : node.children) 
			child = &flattened_tree[address_map[child]];
}

behaviour_tree::behaviour* behaviour_tree::tree::retrieve_behaviour(behaviour* real_address) {
	return &flattened_tree[address_map[real_address]];
}


int behaviour_tree::behaviour::tick(task& current_task) {
	if (current_status != status::RUNNING)
		on_enter(current_task);
	
	/* handle script callback */
	current_status = on_update(current_task);
	
	/* traverse further only if script succeeds at this node */
	if (current_status == status::SUCCESS)
		current_status = traverse(current_task);
	
	if (current_status != status::RUNNING)
		on_exit(current_task, current_status);
	
	return current_status;
}

void behaviour_tree::behaviour::on_enter(task& current_task) {
	luabind::call_function<void>(enter_callback, current_task.subject);
}

void behaviour_tree::behaviour::on_exit(task& current_task, int exit_code) {
	luabind::call_function<void>(exit_callback, current_task.subject, exit_code);
}

int behaviour_tree::behaviour::on_update(task& current_task) {
	return luabind::call_function<int>(update_callback, current_task.subject);
}

int behaviour_tree::behaviour::traverse(task& current_task) {
	if (node_type == type::SEQUENCER) {
		for (auto& child : children) {
			auto exit_code = child->tick(current_task);

			if (exit_code != status::SUCCESS)
				return exit_code;
		}
		
		/* all succedeed */
		return status::SUCCESS;
	}
	
	else if (node_type == type::SELECTOR) {
		for (auto& child : children) {
			auto exit_code = child->tick(current_task);
			
			if (exit_code != status::FAILURE) 
				return exit_code;
		}
		
		/* none succedeed */
		return status::FAILURE;
	}

	assert(0);
	return status::FAILURE;
}

void components::behaviour_tree::behaviour::reset_subtree(const task& current_task) {
	current_status = status::INVALID;
	
	if (current_task.running_parent_node == this) {
		current_status = status::RUNNING;
		children.at(current_task.running_index)->current_status = status::RUNNING;
	}
	
	for (auto& child : children) 
		reset_subtree(current_task);
}

void behaviour_tree_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& tree = it->get<components::behaviour_tree>();
		tree.starting_node->reset_subtree(tree.task_instance);
		
		tree.task_instance.subject = it;
		tree.starting_node->tick(tree.task_instance);
	}
}

#include <gtest\gtest.h>

TEST(BehaviourTree, InvalidOnInit) {
	behaviour_tree::behaviour my_behaviour;
	EXPECT_EQ(my_behaviour.current_status, my_behaviour.INVALID);
	EXPECT_EQ(my_behaviour.node_type, my_behaviour.SEQUENCER);
}

TEST(BehaviourTree, DepthFirstOrderAndMultipleParents) {
	/*
	numbers are connected so they form depth-first traversal order
	0 ->1
	1 ->2
	2 ->3
	2 ->4
	1 ->5
	5 ->6
	5 ->7
	5 ->8
	0 ->9
	0 ->10
	0 ->11
	11->12
	11->13
	13->5
	11->14
	14->5
	*/

#define NODE_COUNT 15
	behaviour_tree::behaviour my_behaviours[NODE_COUNT];
	my_behaviours[0].children.push_back(my_behaviours +	 1);
	my_behaviours[1].children.push_back(my_behaviours +	 2);
	my_behaviours[2].children.push_back(my_behaviours +	 3);
	my_behaviours[2].children.push_back(my_behaviours +	 4);
	my_behaviours[1].children.push_back(my_behaviours +	 5);
	my_behaviours[5].children.push_back(my_behaviours +	 6);
	my_behaviours[5].children.push_back(my_behaviours +	 7);
	my_behaviours[5].children.push_back(my_behaviours +	 8);
	my_behaviours[0].children.push_back(my_behaviours +	 9);
	my_behaviours[0].children.push_back(my_behaviours +	 10);
	my_behaviours[0].children.push_back(my_behaviours +	 11);
	my_behaviours[11].children.push_back(my_behaviours + 12);
	my_behaviours[11].children.push_back(my_behaviours + 13);
	my_behaviours[11].children.push_back(my_behaviours + 13);

	/* a connection to already parented node */
	my_behaviours[13].children.push_back(my_behaviours + 5);
	my_behaviours[11].children.push_back(my_behaviours + 14);
	/* a connection to already parented node */
	my_behaviours[14].children.push_back(my_behaviours + 5);
	
	/* create some means of identification */
	for (int i = 0; i < NODE_COUNT; ++i) {
		my_behaviours[i].node_type = i;
	}

	behaviour_tree::tree my_tree;
	my_tree.create_flattened_tree(my_behaviours);

	ASSERT_EQ(my_tree.flattened_tree.size(), NODE_COUNT);
	EXPECT_EQ(my_tree.address_map.size(), NODE_COUNT);

	for (size_t i = 0; i < NODE_COUNT; ++i)
		EXPECT_EQ(my_tree.flattened_tree[i].node_type, i);

	for (size_t i = 0; i < NODE_COUNT; ++i) {
		EXPECT_EQ(my_tree.address_map[my_behaviours + i], i);
		EXPECT_EQ(my_tree.retrieve_behaviour(my_behaviours + i), &my_tree.flattened_tree[i]);
	}
}


TEST(BehaviourTree, DepthFirstOrderAndMultipleParents) {



}