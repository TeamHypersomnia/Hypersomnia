#include "stdafx.h"
#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

using namespace components;

behaviour_tree::task::task() : subject(nullptr), running_parent_node(nullptr), running_index(0u) {}

bool behaviour_tree::task::operator==(const task& b) const {
	return running_parent_node == b.running_parent_node && running_index == b.running_index;
}

behaviour_tree::composite::composite() : node_type(type::SEQUENCER), default_return(status::RUNNING), decorator_chain(nullptr) {}

//void behaviour_tree::tree::flatten_routine(behaviour* node) {
//	/* depth first order */
//	auto found_entry = address_map.find(node);
//	
//	/* count only if it is unique as it is DAG */
//	if (found_entry == address_map.end()) {
//		address_map[node] = flattened_tree.size();
//		flattened_tree.push_back(*node);
//
//		for (auto child : node->children)
//			flatten_routine(child);
//	}
//}
//
//void behaviour_tree::tree::create_flattened_tree(behaviour* root) {
//	flattened_tree.clear();
//	address_map.clear();
//
//	flatten_routine(root);
//
//	for (auto& node : flattened_tree) 
//		for (auto& child : node.children) 
//			child = &flattened_tree[address_map[child]];
//}

//behaviour_tree::behaviour* behaviour_tree::tree::retrieve_behaviour(behaviour* real_address) {
//	return &flattened_tree[address_map[real_address]];
//}

bool behaviour_tree::composite::is_currently_running(const task& current_task) const {
	if (current_task.running_parent_node) 
		return this == (current_task.running_parent_node->children[current_task.running_index]);

	return false;
}

bool behaviour_tree::composite::is_running_parent(const task& current_task) const {
	return current_task.running_parent_node == this;
}

int behaviour_tree::composite::update(update_input in) {
	if (decorator_chain)
		return decorator_chain->update(this, in);

	return tick(in);
}

int behaviour_tree::composite::tick(update_input in) {
	if (!is_currently_running(*in.current_task))
		on_enter(*in.current_task);
	
	/* handle script callback */
	auto current_status = on_update(*in.current_task);
	
	/* detected leaf that returned RUNNING status, interrupt the other running action */
	if (!is_currently_running(*in.current_task) && current_status == status::RUNNING) {
		interrupt_running(in);
	}
	/* the case where the running node finally succeds 
	is in the case where something different from RUNNING was returned to the node */

	/* traverse further only if script succeeds at this node */
	if (current_status == status::SUCCESS) {
		current_status = traverse(*in.current_task);
	}
	
	/* end this node if it was running but has just finalized */
	if (is_currently_running(*in.current_task) && current_status != status::RUNNING) {
		interrupt_running(update_input(in.current_task), current_status);
	}
	/* else call on_exit for a regular finalized node */
	else if (!is_currently_running(*in.current_task)) {
		on_exit(*in.current_task, current_status);
	}
	
	return current_status;
}

void behaviour_tree::composite::on_enter(task& current_task) {
	if (enter_callback) {
		std::cout << "entering " << name << std::endl;
		try {
			luabind::call_function<void>(enter_callback, current_task.subject);
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << std::endl;
		}
	}
}

void behaviour_tree::composite::on_exit(task& current_task, int exit_code) {
	if (exit_callback) {
		std::cout << "quitting " << name << std::endl;
		try {
			luabind::call_function<void>(exit_callback, current_task.subject, exit_code);
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << std::endl;
		}
	}
}

int behaviour_tree::composite::on_update(task& current_task) {
	if (update_callback) {
		//std::cout << "updating " << name << std::endl;
		try {
			auto result = luabind::call_function<int>(update_callback, current_task.subject);
			return result;  
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << std::endl;
		}
	}

	return default_return;
}

void behaviour_tree::composite::interrupt_running(update_input in, int exit_code) {
	if (in.current_task->running_parent_node) 
		in.current_task->running_parent_node->children[in.current_task->running_index]->on_exit(*in.current_task, exit_code);
	
	in.current_task->running_index = in.child_index;
	in.current_task->running_parent_node = in.parent;
	in.current_task->since_entered.reset();
}

int behaviour_tree::composite::traverse(task& current_task) {
	size_t beginning_node = (this == current_task.running_parent_node) ? current_task.running_index : 0u;
	update_input in(&current_task);

	if (node_type == type::SEQUENCER) {
		for (size_t i = beginning_node; i < children.size(); ++i) {
			in.child_index = i;
			in.parent = this;
			auto exit_code = children[i]->update(in);

			if (exit_code != status::SUCCESS)
				return exit_code;
		}
		
		/* all succedeed */
		return status::SUCCESS;
	}
	
	else if (node_type == type::SELECTOR) {
		if (children.empty()) return status::SUCCESS;

		for (size_t i = beginning_node; i < children.size(); ++i) {
			in.child_index = i;
			in.parent = this;
			auto exit_code = children[i]->update(in);

			if (exit_code != status::FAILURE) 
				return exit_code;
		}
		
		/* none succedeed */
		return status::FAILURE;
	}

	assert(0);
	return status::FAILURE;
}

void behaviour_tree::composite::add_child(composite* b) {
	children.push_back(b);
}

int behaviour_tree::composite::begin_traversal(task& current_task) {
	update_input in;
	in.current_task = &current_task;
	auto status = update(in);
	
	if (status != behaviour_tree::behaviour::RUNNING)
		interrupt_running(in);

	return status;
}

void behaviour_tree_system::process_entities(world& owner) {
	substep(owner);
}

void behaviour_tree_system::substep(world& owner) {
	for (auto it : targets) {
		auto& tree = it->get<components::behaviour_tree>();
		
		tree.task_instance.subject = it;

		/* if the root returns value different from RUNNING, it means that the traversal did not reach the running node
		and thus the running node has to be interrupted
		*/
		tree.starting_node->begin_traversal(tree.task_instance);
	}
}

//behaviour_tree::decorator::decorator() : base_node(nullptr) {}
behaviour_tree::timer_decorator::timer_decorator() : maximum_running_time_ms(0.f) {
	int breakp = 22;
}

//int behaviour_tree::decorator::tick(task& t, composite* c, size_t s) {
//	return base_node->tick(t, c, s);
//}

//int behaviour_tree::decorator::traverse(task& t) {
//	return base_node->traverse(t);
//}
//
//void behaviour_tree::decorator::on_enter(task& current_task) {
//	base_node->on_enter(current_task);
//}
//
//void behaviour_tree::decorator::on_exit(task& current_task, int exit_code) {
//	base_node->on_exit(current_task, exit_code);
//}
//
//int behaviour_tree::decorator::on_update(task& current_task) {
//	return base_node->on_update(current_task);
//}
//
//void behaviour_tree::timer_decorator::on_enter(task& current_task) {
//	timer.reset();
//	decorator::on_enter(current_task);
//}

//int behaviour_tree::timer_decorator::on_update(task& current_task) {
//	if (timer.get<std::chrono::milliseconds>() < maximum_running_time_ms)
//		return decorator::on_update(current_task);
//	else return status::SUCCESS;
//}

int behaviour_tree::decorator::update(composite* current, composite::update_input in) {
	if (next_decorator) 
		return next_decorator->update(current, in);
	return current->tick(in);
}

int behaviour_tree::timer_decorator::update(composite* current, composite::update_input in) {
	int result = decorator::update(current, in);

	if (result != composite::RUNNING) 
		return result;
	
	if (in.current_task->since_entered.get<std::chrono::milliseconds>() > maximum_running_time_ms)
		return composite::SUCCESS;
	else return composite::RUNNING;
}




#include <gtest\gtest.h>

#define NODE_COUNT 15

TEST(BehaviourTree, SequencerFailureSuccess) {
	behaviour_tree::composite my_behaviours[5];
	for (int i = 0; i < 5; ++i) {
		my_behaviours[i].node_type = behaviour_tree::composite::SEQUENCER;
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

	//behaviour_tree::tree my_tree;
	//my_tree.create_flattened_tree(my_behaviours);

	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[3].default_return = behaviour_tree::behaviour::FAILURE;
	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
}

TEST(BehaviourTree, SelectorFailureSuccess) {
	behaviour_tree::composite my_behaviours[5];
	for (int i = 0; i < 5; ++i) {
		my_behaviours[i].node_type = behaviour_tree::composite::SELECTOR;
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

	//behaviour_tree::tree my_tree;
	//my_tree.create_flattened_tree(my_behaviours);

	EXPECT_EQ(behaviour_tree::behaviour::FAILURE, my_behaviours[0].begin_traversal(my_task));
	my_behaviours[3].default_return = behaviour_tree::behaviour::SUCCESS;
	EXPECT_EQ(behaviour_tree::behaviour::SUCCESS, my_behaviours[0].begin_traversal(my_task));
}

struct mock_behaviour : behaviour_tree::composite {
	int enter_called;
	int exit_called;
	int update_called;

	mock_behaviour() : enter_called(0), exit_called(0), update_called(0) {}

	void on_enter(behaviour_tree::task& t) override {
		++enter_called;
		return behaviour_tree::composite::on_enter(t);
	}

	void on_exit(behaviour_tree::task& t, int exit_code) override {
		++exit_called;
		return behaviour_tree::composite::on_exit(t, exit_code);
	}

	int on_update(behaviour_tree::task& t) override {
		++update_called;
		return behaviour_tree::composite::on_update(t);
	}
};

TEST(BehaviourTree, TwoSequentialRunningExitCount) {
	mock_behaviour my_behaviours[5];
	my_behaviours[0].children.push_back(my_behaviours + 1);
	my_behaviours[0].children.push_back(my_behaviours + 2);
	my_behaviours[0].children.push_back(my_behaviours + 3);
	my_behaviours[0].children.push_back(my_behaviours + 4);
	
	for (int i = 0; i < 5; ++i) {
		my_behaviours[i].node_type = behaviour_tree::composite::SEQUENCER;
		my_behaviours[i].default_return = behaviour_tree::behaviour::SUCCESS;
	}

	my_behaviours[2].default_return = behaviour_tree::behaviour::RUNNING;

	behaviour_tree::task my_task;
	my_behaviours[0].begin_traversal(my_task);

	my_behaviours[2].default_return = behaviour_tree::behaviour::SUCCESS;
	my_behaviours[3].default_return = behaviour_tree::behaviour::RUNNING;
	my_behaviours[0].begin_traversal(my_task);

	EXPECT_EQ(2, my_behaviours[0].enter_called);
	EXPECT_EQ(2, my_behaviours[0].update_called);
	EXPECT_EQ(2, my_behaviours[0].exit_called);
	
	EXPECT_EQ(1, my_behaviours[1].enter_called);
	EXPECT_EQ(1, my_behaviours[1].update_called);
	EXPECT_EQ(1, my_behaviours[1].exit_called);
	
	EXPECT_EQ(1, my_behaviours[2].enter_called);
	EXPECT_EQ(2, my_behaviours[2].update_called);
	EXPECT_EQ(1, my_behaviours[2].exit_called);

	EXPECT_EQ(1, my_behaviours[3].enter_called);
	EXPECT_EQ(1, my_behaviours[3].update_called);
	EXPECT_EQ(0, my_behaviours[3].exit_called);

	EXPECT_EQ(0, my_behaviours[4].enter_called);
	EXPECT_EQ(0, my_behaviours[4].update_called);
	EXPECT_EQ(0, my_behaviours[4].exit_called);
}

TEST(BehaviourTree, RunningInterruptions) {


	mock_behaviour my_behaviours[NODE_COUNT];
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
		my_behaviours[i].node_type = behaviour_tree::composite::SEQUENCER;
		my_behaviours[i].default_return = behaviour_tree::behaviour::SUCCESS;
	}
	my_behaviours[12].default_return = behaviour_tree::behaviour::RUNNING;

	behaviour_tree::task my_task;
	my_behaviours[0].begin_traversal(my_task);

	/* check if all nodes before the running node were correctly traversed once */
	for (int i = 0; i < 12; ++i) {
		EXPECT_EQ(1, my_behaviours[i].enter_called);
		EXPECT_EQ(1, my_behaviours[i].update_called);
		EXPECT_EQ(1, my_behaviours[i].exit_called);
	}

	/* check if the running node did not exit */
	EXPECT_EQ(1, my_behaviours[12].enter_called);
	EXPECT_EQ(1, my_behaviours[12].update_called);
	EXPECT_EQ(0, my_behaviours[12].exit_called);

	/* check if the past-running nodes were not traversed */
	for (int i = 13; i < 15; ++i) {
		EXPECT_EQ(0, my_behaviours[i].enter_called);
		EXPECT_EQ(0, my_behaviours[i].update_called);
		EXPECT_EQ(0, my_behaviours[i].exit_called);
	}

	/* check if the running node was correctly picked */
	EXPECT_EQ(my_behaviours + 11, my_task.running_parent_node);
	EXPECT_EQ(0, my_task.running_index);

	my_behaviours[4].default_return = behaviour_tree::behaviour::RUNNING;
	
	my_behaviours[0].begin_traversal(my_task);

	for (int i = 0; i < 4; ++i) {
		EXPECT_EQ(2, my_behaviours[i].enter_called);
		EXPECT_EQ(2, my_behaviours[i].update_called);
		EXPECT_EQ(2, my_behaviours[i].exit_called);
	}

	EXPECT_EQ(2, my_behaviours[4].enter_called);
	EXPECT_EQ(2, my_behaviours[4].update_called);
	EXPECT_EQ(1, my_behaviours[4].exit_called);

	/* check if the previous running correctly quit */
	EXPECT_EQ(1, my_behaviours[12].enter_called);
	EXPECT_EQ(1, my_behaviours[12].update_called);
	EXPECT_EQ(1, my_behaviours[12].exit_called);

	EXPECT_EQ(my_behaviours + 2, my_task.running_parent_node);
	EXPECT_EQ(1, my_task.running_index);
	
	my_behaviours[4].default_return = behaviour_tree::behaviour::FAILURE;
	my_behaviours[12].default_return = behaviour_tree::behaviour::FAILURE;
	
	my_behaviours[0].begin_traversal(my_task);
	EXPECT_EQ(nullptr, my_task.running_parent_node);

	//for (int i = 0; i < NODE_COUNT; ++i) 
	//	EXPECT_EQ(my_behaviours[i].current_status, behaviour_tree::behaviour::INVALID);
}

TEST(BehaviourTree, InitValues) {
	behaviour_tree::composite my_behaviour;
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

	behaviour_tree::composite my_behaviours[NODE_COUNT];
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

	//behaviour_tree::tree my_tree;
	//my_tree.create_flattened_tree(my_behaviours);

	//ASSERT_EQ(NODE_COUNT, my_tree.flattened_tree.size());
	//ASSERT_EQ(NODE_COUNT, my_tree.address_map.size());
	//
	//for (size_t i = 0; i < NODE_COUNT; ++i)
	//	EXPECT_EQ(i, my_tree.flattened_tree[i].node_type);
	//
	//for (size_t i = 0; i < NODE_COUNT; ++i) {
	//	EXPECT_EQ(i, my_tree.address_map[my_behaviours + i]);
	//	EXPECT_EQ(&my_tree.flattened_tree[i], my_tree.retrieve_behaviour(my_behaviours + i));
	//}
}
