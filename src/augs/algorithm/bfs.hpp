#pragma once
#include <optional>
#include <queue>
#include "augs/enums/callback_result.h"

namespace augs {

/*
	Generic BFS algorithm.
	Accepts lambdas for all graph-specific operations.

	Template parameters:
		Id - The type of node identifier (transparent to this algorithm)
		GetVisited - Lambda: (Id) -> bool, returns true if node was visited
		SetVisited - Lambda: (Id) -> void, marks node as visited
		ForEachNeighbor - Lambda: (Id, Callback) -> void, calls callback with each neighbor's Id
		                  Callback should return callback_result::CONTINUE or ABORT
		OnNodeVisit - Lambda: (Id, Id) -> bool, called when visiting a node (current, first_step_from_start)
		              returns true to stop BFS early (target found)

	Returns: std::optional<Id> - the first step from start towards target, or nullopt if not found
*/

template <class Id>
using bfs_queue_type = std::queue<std::pair<Id, Id>>;

template <
	class Id,
	class Queue,
	class GetVisited,
	class SetVisited,
	class ForEachNeighbor,
	class OnNodeVisit
>
std::optional<Id> bfs_find_path(
	Queue& bfs_queue,
	const Id start,
	GetVisited&& get_visited,
	SetVisited&& set_visited,
	ForEachNeighbor&& for_each_neighbor,
	OnNodeVisit&& on_node_visit
) {
	/*
		Clear the queue for reuse.
	*/
	while (!bfs_queue.empty()) {
		bfs_queue.pop();
	}

	/*
		Mark start as visited.
	*/
	set_visited(start);

	/*
		Check if start itself is the target.
	*/
	if (on_node_visit(start, start)) {
		return start;
	}

	/*
		Add all neighbors of start to the queue.
	*/
	std::optional<Id> early_result;

	for_each_neighbor(start, [&](const Id neighbor) {
		if (early_result.has_value()) {
			return callback_result::ABORT;
		}

		if (!get_visited(neighbor)) {
			set_visited(neighbor);

			if (on_node_visit(neighbor, neighbor)) {
				/*
					Target found at first step.
				*/
				early_result = neighbor;
				return callback_result::ABORT;
			}

			bfs_queue.push({ neighbor, neighbor });
		}

		return callback_result::CONTINUE;
	});

	if (early_result.has_value()) {
		return early_result;
	}

	/*
		BFS loop.
	*/
	while (!bfs_queue.empty()) {
		const auto [current, first_step] = bfs_queue.front();
		bfs_queue.pop();

		std::optional<Id> found_result;

		for_each_neighbor(current, [&](const Id neighbor) {
			if (found_result.has_value()) {
				return callback_result::ABORT;
			}

			if (!get_visited(neighbor)) {
				set_visited(neighbor);

				if (on_node_visit(neighbor, first_step)) {
					found_result = first_step;
					return callback_result::ABORT;
				}

				bfs_queue.push({ neighbor, first_step });
			}

			return callback_result::CONTINUE;
		});

		if (found_result.has_value()) {
			return found_result;
		}
	}

	return std::nullopt;
}

} /* namespace augs */
