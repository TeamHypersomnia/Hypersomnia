#include "stdafx.h"
#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

class NullStream {
public:
	NullStream() { }
	template<typename T> NullStream& operator<<(const T &) { return *this; }
};

//#define NDEBUG
#ifdef NDEBUG
	#define COUT std::cout
#else
	#define COUT NullStream()
#endif

using namespace components;

behaviour_tree::task::task() : subject(nullptr), running_parent_node(nullptr), running_index(0u) {}

bool behaviour_tree::task::operator==(const task& b) const {
	return running_parent_node == b.running_parent_node && running_index == b.running_index;
}

behaviour_tree::composite::composite() : node_type(type::SEQUENCER), default_return(status::RUNNING), decorator_chain(nullptr), concurrent_return(status::RUNNING), skip_to_running_child(true) {}

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
		set_running(in);
	}
	/* the case where the running node finally succeds 
	is in the case where something different from RUNNING was returned to the node */

	/* traverse further only if script succeeds at this node */
	if (current_status == status::SUCCESS) {
		current_status = traverse(*in.current_task);
	}
	
	/* end this node if it was running but has just finalized */
	if (is_currently_running(*in.current_task) && current_status != status::RUNNING) {
		in.current_task->interrupt_runner(current_status);
	}
	/* else call on_exit for a regular finalized node */
	else if (!is_currently_running(*in.current_task)) {
		on_exit(*in.current_task, current_status);
	}
	
	return current_status;
}

std::string behaviour_tree::composite::get_type_str() const {
	std::string node_type_name;

	if (children.empty()) node_type_name = "LEAF";
	else switch (node_type) {
	case SELECTOR: node_type_name = "SELECTOR"; break;
	case SEQUENCER: node_type_name = "SEQUENCER"; break;
	case CONCURRENT: node_type_name = "CONCURRENT"; break;
	default: node_type_name = "UNKNOWN"; break;
	}

	return node_type_name;
}

std::string behaviour_tree::composite::get_result_str(int result) const {
	std::string result_str;
	switch (result) {
		case status::SUCCESS: result_str = "SUCCESS"; break;
		case status::FAILURE: result_str = "FAILURE"; break;
		case status::RUNNING: result_str = "RUNNING"; break;
		default: result_str = "UNKNOWN"; break;
	}

	return result_str;
}

void behaviour_tree::composite::on_enter(task& current_task) {
	COUT << "entering " << name << " which is " << get_type_str() << '\n';
	if (enter_callback) {
		try {
			luabind::call_function<void>(enter_callback, current_task.subject, &current_task);
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << '\n';
		}
	}
}

void behaviour_tree::composite::on_exit(task& current_task, int exit_code) {
	if (exit_callback) {
		try {
			luabind::call_function<void>(exit_callback, current_task.subject, exit_code);
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << '\n';
		}
	}
	COUT << "quitting " << name << " which was " << get_type_str() << " and resulted in " << get_result_str(exit_code) << '\n';
}

int behaviour_tree::composite::on_update(task& current_task) {
	if (update_callback) {
		try {
			int result = luabind::call_function<int>(update_callback, current_task.subject);
			COUT << "updating " << name << " which is " << get_type_str() << " results in " << get_result_str(result) << '\n';
			return result;
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << '\n';
		}
	}

	COUT << "updating " << name << " which is " << get_type_str() << " returns by default " << get_result_str(default_return) << '\n';
	return default_return;
}

void behaviour_tree::composite::set_running(update_input in, int exit_code) {
	in.current_task->interrupt_runner(exit_code);

	COUT << "setting " << in.parent->children[in.child_index]->name << "as currently RUNNING" << '\n';
	in.current_task->running_index = in.child_index;
	in.current_task->running_parent_node = in.parent;
	in.current_task->since_entered.reset();
}

void behaviour_tree::task::interrupt_runner(int exit_code) {
	if (running_parent_node) {
		COUT << "interrupting " << running_parent_node->children[running_index]->name << '\n';
		running_parent_node->children[running_index]->on_exit(*this, exit_code);
	}
	
	running_index = 0;
	running_parent_node = nullptr;
	since_entered.reset();
}

int behaviour_tree::composite::traverse(task& current_task) {
	size_t beginning_node = (this == current_task.running_parent_node && skip_to_running_child) ? current_task.running_index : 0u;
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
	else if (node_type == type::CONCURRENT) {
		for (size_t i = beginning_node; i < children.size(); ++i) {
			in.child_index = i;
			in.parent = this;
			auto exit_code = children[i]->update(in);

			if (exit_code == status::RUNNING)
				return exit_code;
		}

		return concurrent_return;
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
	
	/* interrupt the current runner if it the traversal has never reached it */
	if (status != behaviour_tree::behaviour::RUNNING)
		in.current_task->interrupt_runner();

	return status;
}

void behaviour_tree_system::process_entities(world& owner) {
	substep(owner);
}

void behaviour_tree_system::substep(world& owner) {
	for (auto it : targets) {
		auto& trees = it->get<components::behaviour_tree>();
		
		for (auto& tree : trees.trees) {
			tree.task_instance.subject = it;

			/* if the root returns value different from RUNNING, it means that the traversal did not reach the running node
			and thus the running node has to be interrupted
			*/
			tree.starting_node->begin_traversal(tree.task_instance);

			COUT << "\n\n::::::::::::::ANOTHER TREE::::::::::::::\n\n";
		}
	}
}

behaviour_tree::timer_decorator::timer_decorator() : maximum_running_time_ms(-1.f), return_failure_for_the_first_ms(-1.f) {
	int breakp = 22;
}

int behaviour_tree::decorator::update(composite* current, composite::update_input in) {
	if (next_decorator) 
		return next_decorator->update(current, in);
	return current->tick(in);
}

int behaviour_tree::timer_decorator::update(composite* current, composite::update_input in) {
	int result = decorator::update(current, in);

	if (result != composite::RUNNING) 
		return result;
	
	if (in.current_task->since_entered.get<std::chrono::milliseconds>() > maximum_running_time_ms) {
		/* previous call to tick had no chance to know that the node has to be interrupted since it returned RUNNING
			so we have to interrupt now
		*/
		in.current_task->interrupt_runner(composite::SUCCESS);
		return composite::SUCCESS;
	}
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
