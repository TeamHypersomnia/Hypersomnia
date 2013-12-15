#include "stdafx.h"
#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

using namespace components;

behaviour_tree::task::task() : subject(nullptr), running_parent_node(nullptr), running_index(0u) {}

bool behaviour_tree::task::operator==(const task& b) const {
	return running_parent_node == b.running_parent_node && running_index == b.running_index;
}

behaviour_tree::behaviour::behaviour() : node_type(type::SEQUENCER), default_return(status::RUNNING) {}

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

bool behaviour_tree::behaviour::is_currently_running(const task& current_task) const {
	if (current_task.running_parent_node) 
		return this == (current_task.running_parent_node->children[current_task.running_index]);

	return false;
}

int behaviour_tree::behaviour::tick(task& current_task, behaviour* parent, size_t i) {
	if (!is_currently_running(current_task))
		on_enter(current_task);
	
	/* handle script callback */
	auto current_status = on_update(current_task);
	
	/* detected leaf that returned RUNNING status, interrupt the other running action */
	if (!is_currently_running(current_task) && current_status == status::RUNNING) {
		interrupt_running(parent, i, current_task);
	}
	/* the case where the running node finally succeds 
	is in the case where something different from RUNNING was returned to the node */

	/* traverse further only if script succeeds at this node */
	if (current_status == status::SUCCESS) {
		current_status = traverse(current_task);
	}
	
	/* end this node if it was running but has just finalized
		or if it is just a regular finalized node 
	*/
	if (is_currently_running(current_task) && current_status != status::RUNNING
		|| 
		!is_currently_running(current_task)
		)
		on_exit(current_task, current_status);
	
	return current_status;
}

void behaviour_tree::behaviour::on_enter(task& current_task) {
	if (enter_callback)
		luabind::call_function<void>(enter_callback, current_task.subject);
}

void behaviour_tree::behaviour::on_exit(task& current_task, int exit_code) {
	if (exit_callback)
		luabind::call_function<void>(exit_callback, current_task.subject, exit_code);
}

int behaviour_tree::behaviour::on_update(task& current_task) {
	if (update_callback)
		return luabind::call_function<int>(update_callback, current_task.subject);

	return default_return;
}

void behaviour_tree::behaviour::interrupt_running(behaviour* new_task, size_t i, task& current_task) {
	if (current_task.running_parent_node) 
		current_task.running_parent_node->children[current_task.running_index]->on_exit(current_task, status::FAILURE);
	
	current_task.running_index = i;
	current_task.running_parent_node = new_task;
}

int behaviour_tree::behaviour::traverse(task& current_task) {
	size_t beginning_node = (this == current_task.running_parent_node) ? current_task.running_index : 0u;

	if (node_type == type::SEQUENCER) {
		for (size_t i = beginning_node; i < children.size(); ++i) {
			auto exit_code = children[i]->tick(current_task, this, i);

			if (exit_code != status::SUCCESS)
				return exit_code;
		}
		
		/* all succedeed */
		return status::SUCCESS;
	}
	
	else if (node_type == type::SELECTOR) {
		if (children.empty()) return status::SUCCESS;

		for (size_t i = beginning_node; i < children.size(); ++i) {
			auto exit_code = children[i]->tick(current_task, this, i);

			if (exit_code != status::FAILURE) 
				return exit_code;
		}
		
		/* none succedeed */
		return status::FAILURE;
	}

	assert(0);
	return status::FAILURE;
}

void behaviour_tree::behaviour::reset_subtree(const task& current_task) {
	//current_status = status::INVALID;
	//
	//if (current_task.running_parent_node == this) {
	//	current_status = status::RUNNING;
	//	children.at(current_task.running_index)->current_status = status::RUNNING;
	//}
	
	//for (auto& child : children) 
	//	child->reset_subtree(current_task);
}

void behaviour_tree::behaviour::add_child(behaviour* b) {
	children.push_back(b);
}

int behaviour_tree::behaviour::begin_traversal(task& current_task) {
	auto status = tick(current_task, nullptr, 0);
	
	if (status != behaviour_tree::behaviour::RUNNING)
		interrupt_running(0, 0, current_task);

	return status;
}


void behaviour_tree_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& tree = it->get<components::behaviour_tree>();
		tree.starting_node->reset_subtree(tree.task_instance);
		
		tree.task_instance.subject = it;

		/* if the root returns value different from RUNNING, it means that the traversal did not reach the running node
		and thus the running node has to be interrupted
		*/
		tree.starting_node->begin_traversal(tree.task_instance);
	}
}









#include <gtest\gtest.h>

#define NODE_COUNT 15

TEST(BehaviourTree, SequencerFailureSuccess) {
	behaviour_tree::behaviour my_behaviours[5];
	for (int i = 0; i < 5; ++i) {
		my_behaviours[i].node_type = behaviour_tree::behaviour::SEQUENCER;
		my_behaviours[i].default_return = behaviour_tree::behaviour::SUCCESS;
	}

	behaviour_tree::task my_task;

	my_behaviours[0].children.push_back(my_behaviours + 1);
	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[1].default_return = behaviour_tree::behaviour::FAILURE;
	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[1].default_return = behaviour_tree::behaviour::SUCCESS;

	my_behaviours[0].children.push_back(my_behaviours + 2);
	my_behaviours[0].children.push_back(my_behaviours + 3);
	my_behaviours[0].children.push_back(my_behaviours + 4);

	behaviour_tree::tree my_tree;
	my_tree.create_flattened_tree(my_behaviours);

	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[3].default_return = behaviour_tree::behaviour::FAILURE;
	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
}

TEST(BehaviourTree, SelectorFailureSuccess) {
	behaviour_tree::behaviour my_behaviours[5];
	for (int i = 0; i < 5; ++i) {
		my_behaviours[i].node_type = behaviour_tree::behaviour::SELECTOR;
		my_behaviours[i].default_return = behaviour_tree::behaviour::FAILURE;
	}

	behaviour_tree::task my_task;
	my_behaviours[0].default_return = behaviour_tree::behaviour::SUCCESS;
	my_behaviours[0].children.push_back(my_behaviours + 1);
	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[1].default_return = behaviour_tree::behaviour::SUCCESS;
	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[1].default_return = behaviour_tree::behaviour::FAILURE;

	my_behaviours[0].children.push_back(my_behaviours + 2);
	my_behaviours[0].children.push_back(my_behaviours + 3);
	my_behaviours[0].children.push_back(my_behaviours + 4);

	behaviour_tree::tree my_tree;
	my_tree.create_flattened_tree(my_behaviours);

	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[3].default_return = behaviour_tree::behaviour::SUCCESS;
	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
}

TEST(BehaviourTree, RunningInterruptions) {
	behaviour_tree::behaviour my_behaviours[NODE_COUNT];
	my_behaviours[0].children.push_back(my_behaviours + 1);
	my_behaviours[1].children.push_back(my_behaviours + 2);
	my_behaviours[2].children.push_back(my_behaviours + 3);
	my_behaviours[2].children.push_back(my_behaviours + 4);
	my_behaviours[1].children.push_back(my_behaviours + 5);
	my_behaviours[5].children.push_back(my_behaviours + 6);
	my_behaviours[5].children.push_back(my_behaviours + 7);
	my_behaviours[5].children.push_back(my_behaviours + 8);
	my_behaviours[0].children.push_back(my_behaviours + 9);
	my_behaviours[0].children.push_back(my_behaviours + 10);
	my_behaviours[0].children.push_back(my_behaviours + 11);
	my_behaviours[11].children.push_back(my_behaviours + 12);
	my_behaviours[11].children.push_back(my_behaviours + 13);
	my_behaviours[11].children.push_back(my_behaviours + 13);
	my_behaviours[13].children.push_back(my_behaviours + 5);
	my_behaviours[11].children.push_back(my_behaviours + 14);
	my_behaviours[14].children.push_back(my_behaviours + 5);

	for (int i = 0; i < NODE_COUNT; ++i) {
		my_behaviours[i].node_type = behaviour_tree::behaviour::SEQUENCER;
		my_behaviours[i].default_return = behaviour_tree::behaviour::SUCCESS;
	}
	my_behaviours[12].default_return = behaviour_tree::behaviour::RUNNING;

	behaviour_tree::tree my_tree;
	my_tree.create_flattened_tree(my_behaviours);

	behaviour_tree::task my_task;
	my_behaviours[0].reset_subtree(my_task);
	my_behaviours[0].begin_traversal(my_task);
	
	//for (int i = 0; i < NODE_COUNT; ++i) {
	//	if (i == 12 || i == 11 || i == 0) EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::RUNNING);
	//	else EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::SUCCESS);
	//}
	
	my_behaviours[0].reset_subtree(my_task);
	EXPECT_EQ(my_behaviours + 11, my_task.running_parent_node);
	EXPECT_EQ(0, my_task.running_index);

	//for (int i = 0; i < NODE_COUNT; ++i) {
	//	if (i == 12 || i == 11) EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::RUNNING);
	//	else EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::INVALID);
	//}

	my_behaviours[4].default_return = behaviour_tree::behaviour::RUNNING;
	
	my_behaviours[0].begin_traversal(my_task);
	my_behaviours[0].reset_subtree(my_task);

	EXPECT_EQ(my_behaviours + 2, my_task.running_parent_node);
	EXPECT_EQ(1, my_task.running_index);
	
	my_behaviours[4].default_return = behaviour_tree::behaviour::FAILURE;
	my_behaviours[12].default_return = behaviour_tree::behaviour::FAILURE;
	
	my_behaviours[0].begin_traversal(my_task);
	my_behaviours[0].reset_subtree(my_task);
	EXPECT_EQ(nullptr, my_task.running_parent_node);

	//for (int i = 0; i < NODE_COUNT; ++i) 
	//	EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::INVALID);
}

TEST(BehaviourTree, InitValues) {
	behaviour_tree::behaviour my_behaviour;
	//EXPECT_EQ(my_behaviour.current_status, my_behaviour.INVALID);
	EXPECT_EQ(my_behaviour.SEQUENCER, my_behaviour.node_type);
	EXPECT_EQ(my_behaviour.RUNNING, my_behaviour.default_return);
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

	ASSERT_EQ(NODE_COUNT, my_tree.flattened_tree.size());
	ASSERT_EQ(NODE_COUNT, my_tree.address_map.size());

	for (size_t i = 0; i < NODE_COUNT; ++i)
		EXPECT_EQ(i, my_tree.flattened_tree[i].node_type);

	for (size_t i = 0; i < NODE_COUNT; ++i) {
		EXPECT_EQ(i, my_tree.address_map[my_behaviours + i]);
		EXPECT_EQ(&my_tree.flattened_tree[i], my_tree.retrieve_behaviour(my_behaviours + i));
	}
}
