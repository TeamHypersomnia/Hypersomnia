#pragma once
#include <optional>
#include <queue>

/*
	Generic BFS algorithm.
	Accepts lambdas for all graph-specific operations.

	Template parameters:
		Id - The type of node identifier (transparent to this algorithm)
		GetVisited - Lambda: (Id) -> bool, returns true if node was visited
		SetVisited - Lambda: (Id) -> void, marks node as visited
		ForEachNeighbor - Lambda: (Id, Callback) -> void, calls callback with each neighbor's Id
		OnNodeVisit - Lambda: (Id, Id) -> bool, called when visiting a node (current, first_step_from_start)
		              returns true to stop BFS early (target found)

	Returns: std::optional<Id> - the first step from start towards target, or nullopt if not found
*/

template <
	class Id,
	class GetVisited,
	class SetVisited,
	class ForEachNeighbor,
	class OnNodeVisit
>
std::optional<Id> bfs_find_path(
	const Id start,
	GetVisited&& get_visited,
	SetVisited&& set_visited,
	ForEachNeighbor&& for_each_neighbor,
	OnNodeVisit&& on_node_visit
) {
	/*
		BFS queue: pair of (current_id, first_step_id)
		first_step_id tracks what node we went to first from start.
	*/
	std::queue<std::pair<Id, Id>> bfs_queue;

	/*
		Mark start as visited.
	*/
	set_visited(start);

	/*
		Add all neighbors of start to the queue.
	*/
	for_each_neighbor(start, [&](const Id neighbor) {
		if (!get_visited(neighbor)) {
			set_visited(neighbor);

			if (on_node_visit(neighbor, neighbor)) {
				/*
					Early exit signal - target found at first step.
				*/
				return;
			}

			bfs_queue.push({ neighbor, neighbor });
		}
	});

	/*
		If the queue is empty and we got here, check if start was the target.
	*/
	if (bfs_queue.empty()) {
		return std::nullopt;
	}

	/*
		BFS loop.
	*/
	while (!bfs_queue.empty()) {
		const auto [current, first_step] = bfs_queue.front();
		bfs_queue.pop();

		bool found = false;
		Id found_first_step = first_step;

		for_each_neighbor(current, [&](const Id neighbor) {
			if (found) {
				return;
			}

			if (!get_visited(neighbor)) {
				set_visited(neighbor);

				if (on_node_visit(neighbor, first_step)) {
					found = true;
					found_first_step = first_step;
					return;
				}

				bfs_queue.push({ neighbor, first_step });
			}
		});

		if (found) {
			return found_first_step;
		}
	}

	return std::nullopt;
}
