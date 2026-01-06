#pragma once
#include <optional>
#include <queue>
#include <vector>
#include <functional>
#include "augs/enums/callback_result.h"

namespace augs {

	/*
		Generic A* algorithm.
		Accepts lambdas for all grid-specific operations.
		Assumes uniform movement cost (1.0) for all edges.

		Template parameters:
		Id - The type of node identifier (transparent to this algorithm)
		GetVisited - Lambda: (Id) -> bool, returns true if node was visited
		SetVisited - Lambda: (Id) -> void, marks node as visited
		ForEachNeighbor - Lambda: (Id, Callback) -> void, calls callback with each neighbor's Id
		Callback should return callback_result::CONTINUE or ABORT
		Heuristic - Lambda: (Id) -> float, returns estimated distance to target
		IsTarget - Lambda: (Id) -> bool, returns true if this is the target node
		GetParent - Lambda: (Id) -> std::optional<Id>, returns parent of node (for path reconstruction)
		SetParent - Lambda: (Id, Id) -> void, sets parent of first arg to second arg
		GetGCost - Lambda: (Id) -> float, returns g-cost (distance from start)
		SetGCost - Lambda: (Id, float) -> void, sets g-cost

		Returns: bool - true if path was found, false otherwise
		Path can be reconstructed using GetParent starting from target.
	*/

	template <class Id>
	struct astar_queue_node {
		Id id;
		float f_cost;

		bool operator>(const astar_queue_node& other) const {
			return f_cost > other.f_cost;
		}
	};

	template <class Id>
	using astar_queue_type = std::priority_queue<
		astar_queue_node<Id>,
		std::vector<astar_queue_node<Id>>,
		std::greater<astar_queue_node<Id>>
	>;

	template <
		class Id,
		class Queue,
		class GetVisited,
		class SetVisited,
		class ForEachNeighbor,
		class Heuristic,
		class IsTarget,
		class SetParent,
		class GetGCost,
		class SetGCost
	>
	bool astar_find_path(
		Queue& open_set,
		const Id start,
		GetVisited&& get_visited,
		SetVisited&& set_visited,
		ForEachNeighbor&& for_each_neighbor,
		Heuristic&& heuristic,
		IsTarget&& is_target,
		SetParent&& set_parent,
		GetGCost&& get_g_cost,
		SetGCost&& set_g_cost
	) {
		/*
			Clear the queue for reuse.
		*/
		while (!open_set.empty()) {
			open_set.pop();
		}

		const auto start_h = heuristic(start);
		set_g_cost(start, 0.0f);
		open_set.push({ start, start_h });

		while (!open_set.empty()) {
			const auto current = open_set.top();
			open_set.pop();

			const auto& current_id = current.id;

			/*
				Skip if already visited.
			*/
			if (get_visited(current_id)) {
				continue;
			}

			set_visited(current_id);

			/*
				Check if target reached.
			*/
			if (is_target(current_id)) {
				return true;
			}

			const auto current_g = get_g_cost(current_id);

			/*
				Explore neighbors.
			*/
			for_each_neighbor(current_id, [&](const Id neighbor) {
				if (get_visited(neighbor)) {
					return callback_result::CONTINUE;
				}

				/*
					Uniform cost: 1.0 for all edges.
				*/
				const auto tentative_g = current_g + 1.0f;

				if (tentative_g < get_g_cost(neighbor)) {
					set_g_cost(neighbor, tentative_g);
					set_parent(neighbor, current_id);

					const auto h = heuristic(neighbor);
					open_set.push({ neighbor, tentative_g + h });
				}

				return callback_result::CONTINUE;
			});
		}

		return false;
	}
}
