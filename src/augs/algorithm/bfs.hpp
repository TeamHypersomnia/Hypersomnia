#pragma once
#include <optional>
#include <queue>
#include "augs/enums/callback_result.h"

namespace augs {

	/*
		BFS algorithm for finding next edge to target.
		
		Template parameters:
			Id - The type of node identifier (transparent to this algorithm)
			Queue - The queue type (typically bfs_queue_type<Id>)
			GetVisited - Lambda: (Id) -> bool, returns true if node was visited
			SetVisited - Lambda: (Id) -> void, marks node as visited
			ForEachNeighbor - Lambda: (Id, Callback) -> void, calls callback with each neighbor's Id
			                  Callback should return callback_result::CONTINUE or ABORT

		Returns: std::optional<Id> - the first step from start towards target, or nullopt if not found
	*/

	template <class Id>
	using bfs_queue_type = std::queue<std::pair<Id, Id>>;

	template <
		class Id,
		class Queue,
		class GetVisited,
		class SetVisited,
		class ForEachNeighbor
	>
	std::optional<Id> bfs_find_next_edge(
		Queue& bfs_queue,
		const Id start,
		const Id target,
		GetVisited&& get_visited,
		SetVisited&& set_visited,
		ForEachNeighbor&& for_each_neighbor
	) {
		/*
			Clear the queue for reuse.
		*/
		while (!bfs_queue.empty()) {
			bfs_queue.pop();
		}

		/*
			Check if start is already the target.
		*/
		if (start == target) {
			return start;
		}

		/*
			Mark start as visited.
		*/
		set_visited(start);

		/*
			Process neighbors and BFS loop.
		*/
		std::optional<Id> result;

		auto process_neighbor = [&](const Id first_step, const Id neighbor) {
			if (result.has_value()) {
				return callback_result::ABORT;
			}

			if (!get_visited(neighbor)) {
				set_visited(neighbor);

				if (neighbor == target) {
					result = first_step;
					return callback_result::ABORT;
				}

				bfs_queue.push({ neighbor, first_step });
			}

			return callback_result::CONTINUE;
		};

		/*
			Process start's neighbors (first_step == neighbor for direct neighbors of start).
		*/
		for_each_neighbor(start, [&](const Id neighbor) {
			return process_neighbor(neighbor, neighbor);
		});

		if (result.has_value()) {
			return result;
		}

		/*
			BFS loop.
		*/
		while (!bfs_queue.empty()) {
			const auto [current, first_step] = bfs_queue.front();
			bfs_queue.pop();

			for_each_neighbor(current, [&](const Id neighbor) {
				return process_neighbor(first_step, neighbor);
			});

			if (result.has_value()) {
				return result;
			}
		}

		return std::nullopt;
	}

} /* namespace augs */

namespace augs {

	/*
		Generic BFS algorithm that finds all nodes matching a predicate,
		tracking parent nodes for path reconstruction.

		Template parameters:
			Id - The type of node identifier
			GetVisited - Lambda: (Id) -> bool, returns true if node was visited
			SetVisited - Lambda: (Id) -> void, marks node as visited
			SetParent - Lambda: (Id child, Id parent) -> void, records parent for path reconstruction
			ForEachNeighbor - Lambda: (Id, Callback) -> void, calls callback with each neighbor's Id
			IsTarget - Lambda: (Id) -> bool, returns true if node matches the search criteria

		Returns: std::vector<Id> - all matching nodes found
	*/

	template <
		class Id,
		class GetVisited,
		class SetVisited,
		class SetParent,
		class ForEachNeighbor,
		class IsTarget
	>
	std::vector<Id> bfs_find_all_matching(
		const Id start,
		GetVisited&& get_visited,
		SetVisited&& set_visited,
		SetParent&& set_parent,
		ForEachNeighbor&& for_each_neighbor,
		IsTarget&& is_target
	) {
		std::queue<Id> queue;
		std::vector<Id> results;

		set_visited(start);
		queue.push(start);

		while (!queue.empty()) {
			const auto current = queue.front();
			queue.pop();

			for_each_neighbor(current, [&](const Id neighbor) {
				if (get_visited(neighbor)) {
					return callback_result::CONTINUE;
				}

				set_visited(neighbor);
				set_parent(neighbor, current);

				if (is_target(neighbor)) {
					results.push_back(neighbor);
				}
				else {
					queue.push(neighbor);
				}

				return callback_result::CONTINUE;
			});
		}

		return results;
	}

}
