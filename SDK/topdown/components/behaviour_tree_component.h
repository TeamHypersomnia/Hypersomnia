#pragma once
#include <vector>
#include <unordered_map>
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "../../../utility/timer.h"

namespace components {
	struct behaviour_tree : public augmentations::entity_system::component {
		struct task;
		struct decorator;

		/* behaviour interface */
		struct behaviour {
			enum status {
				RUNNING,
				FAILURE,
				SUCCESS
			};
			/*
			virtuals are for mocking and decorators
			assertions placed instead of just being abstract because of whining luabind
			*/
			//virtual int tick(task&, composite*, size_t) { assert(0);  return status::SUCCESS; }
			//virtual int traverse(task&) { assert(0); return status::SUCCESS; }
			//
			///* these functions only call the script delegates */
			//virtual void on_enter(task&) { assert(0); }
			//virtual void on_exit(task&, int exit_code) { assert(0); }
			//virtual int on_update(task&) { assert(0); return status::SUCCESS; }
		};

		/* base behaviour composite */
		struct composite : behaviour {
			typedef std::vector<composite*> behaviors;

			enum type {
				SEQUENCER,
				SELECTOR
			};

			std::string name;

			int default_return;
			int node_type;
			behaviors children;

			int begin_traversal(task&);

			struct update_input {
				task* current_task;
				composite* parent;
				size_t child_index;
				update_input(task* current_task = nullptr) : current_task(current_task), child_index(0u), parent(nullptr) {}
			};

			int update(update_input);
			virtual int tick(update_input);
			virtual int traverse(task&);
			
			virtual void on_enter(task&);
			virtual void on_exit(task&, int exit_code);
			virtual int on_update(task&);

			bool is_currently_running(const task&) const;
			bool is_running_parent(const task&) const;
			static void interrupt_running(update_input, int status = status::FAILURE);

			/* actual implemented behaviours */
			luabind::object enter_callback;
			luabind::object exit_callback;
			luabind::object update_callback;

			composite();

			void add_child(composite*);

			decorator* decorator_chain;
		};

		/* an inverse decorator to keep the tree structure uninvaded */
		struct decorator {
			decorator* next_decorator;
			decorator() : next_decorator(nullptr) {}

			virtual int update(composite* current, composite::update_input);

			//virtual int tick(task& t, composite* c, size_t s) override { return base_node->tick(t, c, s); }
			//virtual int traverse(task& t) override { return base_node->traverse(t); }
			//
			//virtual void on_enter(task& t) override { base_node->on_enter(t); }
			//virtual void on_exit(task& t, int exit_code) override { base_node->on_exit(t, exit_code); }
			//virtual int on_update(task& t) override { return base_node->on_update(t); }
		};

		struct timer_decorator : decorator {
			float maximum_running_time_ms;
			
			timer_decorator();
			int update(composite* current, composite::update_input) override;
		};


		/* not to be used now, have to go with data blobs */
		//class tree {
		//	void flatten_routine(behaviour_interface* root);
		//public:
		//	std::unordered_map<behaviour*, size_t> address_map;
		//	std::vector<behaviour> flattened_tree;
		//
		//	void create_flattened_tree(behaviour* root);
		//	behaviour* retrieve_behaviour(behaviour*);
		//};

		struct task {
			augmentations::entity_system::entity* subject;

			composite* running_parent_node;
			size_t running_index;

			bool operator==(const task&) const;

			task();
			
			augmentations::util::timer since_entered;
		};

		composite* starting_node;
		task task_instance;
	};
}

