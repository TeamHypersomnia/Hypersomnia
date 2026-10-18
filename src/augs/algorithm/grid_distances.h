#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <array>
#include <utility>
#include <vector>
#include "augs/math/vec2.h"

namespace augs {
	inline constexpr uint32_t grid_distance_unreachable_v = std::numeric_limits<uint32_t>::max();
	inline constexpr uint32_t grid_straight_step_cost_v = 10;
	inline constexpr uint32_t grid_diagonal_step_cost_v = 14;

	/*
		Several grids of cells, their cells numbered one after another -
		a cell's index is its grid's offset plus the index inside that grid.
	*/

	struct grids_layout {
		std::vector<vec2u> sizes;
		std::vector<std::size_t> offsets;
		std::size_t total = 0;

		void add(const vec2u size) {
			sizes.push_back(size);
			offsets.push_back(total);
			total += static_cast<std::size_t>(size.x) * size.y;
		}

		std::size_t grid_of(const std::size_t index) const {
			const auto found = std::upper_bound(offsets.begin(), offsets.end(), index);
			return static_cast<std::size_t>(found - offsets.begin()) - 1;
		}
	};

	/*
		Path distances over the grids - 8 neighbours, 10 per straight step and 14 per diagonal one,
		so integers only, the same on every machine. A diagonal step never cuts the corner of a blocked cell.
		for_each_teleport(cell, callback(target_cell)) lists the cells a cell leads to at the cost of one straight step
		(e.g. a portal to another grid).

		Lowers the distances from the given source cells wherever they are shorter than those already there -
		so calling it again with new sources keeps, in every cell, the distance to the nearest of all sources so far
		(only the cells that got closer are visited again). Fill `distances` with grid_distance_unreachable_v first.
		Sources may be blocked cells themselves (e.g. a spawn standing on a wall's edge) - only the steps out of them must be free.
		With a cutoff, only distances below it are computed (exactly) - farther cells keep what they had,
		which is enough to tell whether something is within a radius and visits far fewer cells.
	*/

	template <class IsWalkable, class ForEachTeleport>
	void lower_grid_distances(
		const grids_layout& layout,
		IsWalkable&& is_walkable,
		ForEachTeleport&& for_each_teleport,
		const std::vector<std::size_t>& sources,
		std::vector<uint32_t>& distances,
		const uint32_t cutoff = grid_distance_unreachable_v
	) {
		/*
			Dial's algorithm: the step costs are small integers, so a cell waiting at distance d sits in bucket d % 15 -
			a step never costs more than 14, so the buckets never hold two different distances at once.
		*/

		constexpr std::size_t num_buckets = grid_diagonal_step_cost_v + 1;

		thread_local std::array<std::vector<std::size_t>, num_buckets> buckets;
		std::size_t pending = 0;

		for (auto& bucket : buckets) {
			bucket.clear();
		}

		for (const auto source : sources) {
			if (source < distances.size() && distances[source] > 0) {
				distances[source] = 0;
				buckets[0].push_back(source);
				++pending;
			}
		}

		auto relax = [&](const std::size_t target, const uint32_t next) {
			if (next < distances[target] && next < cutoff) {
				distances[target] = next;
				buckets[next % num_buckets].push_back(target);
				++pending;
			}
		};

		for (uint32_t current = 0; pending > 0; ++current) {
			auto& bucket = buckets[current % num_buckets];

			while (!bucket.empty()) {
				const auto index = bucket.back();
				bucket.pop_back();
				--pending;

				if (distances[index] != current) {
					continue;
				}

				const auto dist = current;

				const auto grid = layout.grid_of(index);
				const auto size = layout.sizes[grid];
				const auto offset = layout.offsets[grid];
				const auto local = index - offset;

				const auto w = static_cast<int>(size.x);
				const auto h = static_cast<int>(size.y);
				const auto x = static_cast<int>(local % size.x);
				const auto y = static_cast<int>(local / size.x);

				auto global = [&](const int cx, const int cy) {
					return offset + static_cast<std::size_t>(cy * w + cx);
				};

				for (int dy = -1; dy <= 1; ++dy) {
					for (int dx = -1; dx <= 1; ++dx) {
						if (dx == 0 && dy == 0) {
							continue;
						}

						const auto nx = x + dx;
						const auto ny = y + dy;

						if (nx < 0 || ny < 0 || nx >= w || ny >= h) {
							continue;
						}

						const auto neighbour = global(nx, ny);

						if (!is_walkable(neighbour)) {
							continue;
						}

						const bool diagonal = dx != 0 && dy != 0;

						if (diagonal && (!is_walkable(global(nx, y)) || !is_walkable(global(x, ny)))) {
							continue;
						}

						relax(neighbour, dist + (diagonal ? grid_diagonal_step_cost_v : grid_straight_step_cost_v));
					}
				}

				for_each_teleport(index, [&](const std::size_t target) {
					if (target < distances.size()) {
						relax(target, dist + grid_straight_step_cost_v);
					}
				});
			}
		}
	}
}
