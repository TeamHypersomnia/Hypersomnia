#pragma once
#include <vector>
#include <functional>
#include <memory>
#include "entity_system/entity_id.h"

namespace resources {
	class behaviour_tree {
	
	public:
		typedef augs::entity_id user_callback_input;

		enum class goal_availability {
			ALREADY_ACHIEVED,
			CANT_EXECUTE,
			SHOULD_EXECUTE
		};

		enum class execution_occurence {
			FIRST,
			LAST,
			REPEATED
		};

		struct state_of_tree_instance {
			int previously_executed_leaf_id = -1;
			user_callback_input user_input;

			void reset();
		};

		struct state_of_traversal {
			state_of_tree_instance& instance;
			const behaviour_tree& original_tree;
		};

		struct goal_data {

		};

		typedef std::unique_ptr<goal_data> goal_ptr;

		struct determined_goal {
			goal_availability availability;
			goal_ptr data;
		};

		class node {
			int id_in_tree = -1;
			friend class behaviour_tree;

			goal_availability evaluate_node(state_of_traversal&) const;
		public:
			enum class type {
				SEQUENCER,
				SELECTOR,
			} mode = type::SELECTOR;

			std::vector<std::unique_ptr<node>> children;

			virtual determined_goal determine_goal_availability(state_of_traversal&) const;
			virtual void execute_leaf_goal_callback(execution_occurence, goal_ptr, state_of_traversal&) const;
		};

		node root;
		void build_tree();
		void evaluate_instance_of_tree(state_of_tree_instance&) const;

		node& get_node_by_id(int) const;
	private:
		std::vector<node*> node_pointers;

		void dfs(node& p, std::function<void(node&)>);
		void dfs_all(std::function<void(node&)>);
	};
}
