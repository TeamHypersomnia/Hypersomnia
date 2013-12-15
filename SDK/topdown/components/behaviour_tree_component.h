#pragma once
#include <vector>
#include <unordered_map>
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

namespace components {
	struct behaviour_tree : public augmentations::entity_system::component {
		struct task;

		/* base behaviour composite */
		struct behaviour {
			typedef std::vector<behaviour*> behaviors;

			enum status {
				RUNNING,
				FAILURE,
				SUCCESS
			};

			enum type {
				SEQUENCER,
				SELECTOR
			};

			int default_return;
			int node_type;
			behaviors children;

			std::string name;

			int begin_traversal(task&);

			int tick(task&, behaviour*, size_t);
			int traverse(task&);
			
			/* these functions only call the script delegates
				virtuals are only for mocking
			*/
			virtual void on_enter(task&);
			virtual void on_exit(task&, int exit_code);
			virtual int on_update(task&);

			bool is_currently_running(const task&) const;
			static void interrupt_running(behaviour* new_running, size_t new_index, task&, int status = status::FAILURE);

			/* actual implemented behaviours */
			luabind::object enter_callback;
			luabind::object exit_callback;
			luabind::object update_callback;

			behaviour();

			void reset_subtree(const task&);
			void add_child(behaviour*);
		};

		class tree {
			void flatten_routine(behaviour* root);
		public:
			std::unordered_map<behaviour*, size_t> address_map;
			std::vector<behaviour> flattened_tree;

			void create_flattened_tree(behaviour* root);
			behaviour* retrieve_behaviour(behaviour*);
		};

		struct task {
			augmentations::entity_system::entity* subject;

			behaviour* running_parent_node;
			size_t running_index;

			bool operator==(const task&) const;

			task();
		};

		behaviour* starting_node;
		task task_instance;
	};
}

