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
				SUCCESS,
				INVALID
			};

			int current_status;

			enum type {
				SEQUENCER,
				SELECTOR
			};
			
			int node_type;

			int tick(task&);
			int traverse(task&);
			
			/* these functions only call the script delegates */
			void on_enter(task&);
			void on_exit(task&, int exit_code);
			int on_update(task&);

			/* actual implemented behaviours */
			luabind::object enter_callback;
			luabind::object exit_callback;
			luabind::object update_callback;

			behaviour();

			void reset_subtree(const task&);
			behaviors children;

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

			task();
		};

		behaviour* starting_node;
		task task_instance;
	};
}

