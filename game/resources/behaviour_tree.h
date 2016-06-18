#pragma once
#include <vector>
#include <memory>
#include <tuple>
#include <array>

#include "game/entity_id.h"
#include "game/detail/ai/goals.h"

#include "augs/templates.h"
#include "ensure.h"

namespace resources {
	class behaviour_tree {
	
	public:
		typedef entity_id user_callback_input;

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
			state_of_traversal(state_of_tree_instance&, const behaviour_tree&);

			state_of_tree_instance& instance;
			const behaviour_tree& original_tree;
			
			behaviours::goal_tuple resolved_goals;
			std::array<bool, std::tuple_size<decltype(resolved_goals)>::value> goals_set;

			template<class T>
			void set_goal(const T& g) {
				std::get<T>(resolved_goals) = g;
				goals_set.at(index_in_tuple<T, decltype(resolved_goals)>::value) = true;
			}

			template<class T>
			T& get_goal() {
				ensure(goals_set.at(index_in_tuple<T, decltype(resolved_goals)>::value));
				return std::get<T>(resolved_goals);
			}
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

			node* branch();

			template<typename Branch, typename... Branches>
			node* branch(Branch b, Branches... branches) {
				children.emplace_back(b);
				return branch(branches...);
			}

			virtual goal_availability goal_resolution(state_of_traversal&) const;
			virtual void execute_leaf_goal_callback(execution_occurence, state_of_traversal&) const;
		};

		node root;
		void build_tree();
		void evaluate_instance_of_tree(state_of_tree_instance&) const;

		const node& get_node_by_id(int) const;
	private:
		std::vector<const node*> node_pointers;

		void dfs(node& p, std::function<void(node&)>);
		void dfs_all(std::function<void(node&)>);
	};
}
