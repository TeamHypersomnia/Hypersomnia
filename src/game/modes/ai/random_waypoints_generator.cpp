#include <algorithm>
#include <cstddef>
#include <optional>
#include "augs/algorithm/grid_distances.h"
#include "augs/misc/randomization.h"
#include "game/common_state/cosmos_navmesh.h"
#include "game/modes/ai/random_waypoints_generator.h"

/*
	The spacing between waypoints of one kind at density 1 - by path, about what the official maps have between their patrols.
	Pushes gather around the front line, so they may stand closer.
*/

static constexpr real32 base_spacing_px = 1100.0f;
static constexpr real32 push_spacing_mult = 0.6f;

/*
	Rays rating the patrols - at most this many per round in total, so generating every round stays cheap.
*/

static constexpr int max_total_rays = 400;
static constexpr int min_rays_per_patrol = 6;
static constexpr int max_rays_per_patrol = 16;
static constexpr real32 ray_length = 3000.0f;
static constexpr real32 covering_ray_length = 450.0f;

/*
	Where a cell lies between the two sides, in permille: 0 at the own side, 1000 at the other.
*/

static uint32_t calc_front_permille(const uint32_t own, const uint32_t other) {
	const auto sum = static_cast<uint64_t>(own) + other;

	if (sum == 0) {
		return 500;
	}

	return static_cast<uint32_t>(static_cast<uint64_t>(own) * 1000 / sum);
}

/*
	All cells of all nav islands numbered one after another (augs::grids_layout),
	the islands joined by their portals - so paths, and the waypoints, span the whole map.
*/

struct navmesh_cells {
	const cosmos_navmesh& navmesh;
	augs::grids_layout layout;

	/* Flat copies, so that the inner loops never look up the island of a cell. */
	std::vector<uint8_t> values;
	std::vector<vec2> centers;

	navmesh_cells(const cosmos_navmesh& navmesh) : navmesh(navmesh) {
		for (const auto& island : navmesh.islands) {
			const bool valid = island.cell_size > 0 && island.occupied.size() >= static_cast<std::size_t>(island.get_size_in_cells().area());
			layout.add(valid ? island.get_size_in_cells() : vec2u::zero);
		}

		values.reserve(layout.total);
		centers.reserve(layout.total);

		for (std::size_t i = 0; i < navmesh.islands.size(); ++i) {
			const auto& island = navmesh.islands[i];
			const auto size = layout.sizes[i];
			const auto cell_size = static_cast<real32>(island.cell_size);

			for (uint32_t y = 0; y < size.y; ++y) {
				for (uint32_t x = 0; x < size.x; ++x) {
					values.push_back(island.occupied[island.cell_index(vec2u(x, y))]);
					centers.push_back(vec2(island.bound.lt()) + vec2(vec2u(x, y)) * cell_size + vec2::square(cell_size / 2));
				}
			}
		}
	}

	std::size_t island_of(const std::size_t global) const {
		const auto found = std::upper_bound(layout.offsets.begin(), layout.offsets.end(), global);
		return static_cast<std::size_t>(found - layout.offsets.begin()) - 1;
	}

	uint8_t value(const std::size_t global) const {
		return values[global];
	}

	bool walkable(const std::size_t global) const {
		return ::is_cell_walkable(value(global));
	}

	bool free(const std::size_t global) const {
		return ::is_cell_unoccupied(value(global));
	}

	vec2 center(const std::size_t global) const {
		return centers[global];
	}

	std::optional<std::size_t> find(const vec2 pos) const {
		for (std::size_t i = 0; i < navmesh.islands.size(); ++i) {
			const auto& island = navmesh.islands[i];

			if (layout.sizes[i].area() == 0) {
				continue;
			}

			const auto offset = pos - vec2(island.bound.lt());

			if (offset.x < 0.0f || offset.y < 0.0f) {
				continue;
			}

			const auto cell = vec2u(offset / static_cast<real32>(island.cell_size));

			if (island.is_within_bounds(cell)) {
				return layout.offsets[i] + island.cell_index(cell);
			}
		}

		return std::nullopt;
	}

	std::vector<std::size_t> find_free_within(const ltrb area) const {
		std::vector<std::size_t> found;

		for (std::size_t c = 0; c < layout.total; ++c) {
			if (free(c) && area.hover(center(c))) {
				found.push_back(c);
			}
		}

		return found;
	}

	std::vector<std::size_t> find_all(const std::vector<vec2>& positions) const {
		std::vector<std::size_t> cells;

		for (const auto& pos : positions) {
			if (const auto cell = find(pos)) {
				cells.push_back(*cell);
			}
		}

		return cells;
	}

	template <class F>
	void for_each_teleport(const std::size_t global, F&& callback) const {
		const auto cell_value = value(global);

		if (!::is_cell_portal(cell_value)) {
			return;
		}

		const auto i = island_of(global);
		const auto portal_index = static_cast<std::size_t>(cell_value - 2);
		const auto& portals = navmesh.islands[i].portals;

		if (portal_index >= portals.size()) {
			return;
		}

		const auto& portal = portals[portal_index];
		const auto out = static_cast<std::size_t>(portal.out_island_index);

		if (out >= navmesh.islands.size() || layout.sizes[out].area() == 0) {
			return;
		}

		const auto& out_island = navmesh.islands[out];

		if (out_island.is_within_bounds(portal.out_cell_pos)) {
			callback(layout.offsets[out] + out_island.cell_index(portal.out_cell_pos));
		}
	}

	std::vector<uint32_t> distances_from(const std::vector<std::size_t>& sources) const {
		std::vector<uint32_t> distances(layout.total, augs::grid_distance_unreachable_v);
		lower(sources, distances);
		return distances;
	}

	void lower(const std::vector<std::size_t>& sources, std::vector<uint32_t>& distances, const uint32_t cutoff = augs::grid_distance_unreachable_v) const {
		augs::lower_grid_distances(
			layout,
			[&](const std::size_t c) { return walkable(c); },
			[&](const std::size_t c, auto&& callback) { for_each_teleport(c, callback); },
			sources,
			distances,
			cutoff
		);
	}

	/*
		The neighbour cell closest to a target by path, as a direction.
	*/

	real32 calc_facing_towards(const std::size_t global, const std::vector<uint32_t>& towards) const {
		const auto i = island_of(global);
		const auto size = layout.sizes[i];
		const auto offset = layout.offsets[i];
		const auto local = global - offset;
		const auto x = static_cast<int>(local % size.x);
		const auto y = static_cast<int>(local / size.x);

		auto best_dir = vec2(1, 0);
		auto best_distance = towards[global];

		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				const auto nx = x + dx;
				const auto ny = y + dy;

				if ((dx == 0 && dy == 0) || nx < 0 || ny < 0 || nx >= static_cast<int>(size.x) || ny >= static_cast<int>(size.y)) {
					continue;
				}

				const auto neighbour = offset + static_cast<std::size_t>(ny * static_cast<int>(size.x) + nx);

				if (towards[neighbour] < best_distance) {
					best_distance = towards[neighbour];
					best_dir = vec2(static_cast<real32>(dx), static_cast<real32>(dy));
				}
			}
		}

		return best_dir.degrees();
	}
};

patrol_spot_rating rate_patrol_spot(
	const vec2 pos,
	const int num_rays,
	const std::function<real32(vec2 from, vec2 to)>& free_distance
) {
	patrol_spot_rating rating;

	if (num_rays <= 0 || !free_distance) {
		return rating;
	}

	std::vector<real32> lengths(static_cast<std::size_t>(num_rays));

	auto ray_degrees = [&](const int r) {
		return 360.0f * static_cast<real32>(r) / static_cast<real32>(num_rays);
	};

	auto best_ray = 0;

	for (int r = 0; r < num_rays; ++r) {
		lengths[r] = free_distance(pos, pos + vec2::from_degrees(ray_degrees(r)) * ray_length);

		if (lengths[r] > lengths[best_ray]) {
			best_ray = r;
		}
	}

	rating.facing = ray_degrees(best_ray);
	rating.view_ahead = lengths[best_ray];

	auto behind = 0;
	auto covered_behind = 0;

	for (int r = 0; r < num_rays; ++r) {
		const auto off_facing = vec2::from_degrees(ray_degrees(r)).degrees_between(vec2::from_degrees(rating.facing));

		if (off_facing > 112.5f) {
			++behind;

			if (lengths[r] < covering_ray_length) {
				++covered_behind;
			}
		}
	}

	const auto openness_ahead = rating.view_ahead / ray_length;
	const auto cover_behind = behind > 0 ? static_cast<real32>(covered_behind) / static_cast<real32>(behind) : 0.0f;

	rating.quality = openness_ahead * cover_behind;
	return rating;
}

std::vector<random_waypoint> generate_random_waypoints(const random_waypoints_input& in) {
	std::vector<random_waypoint> output;

	if (in.navmesh == nullptr) {
		return output;
	}

	auto rng = randomization(in.seed);
	const auto density = std::clamp(in.density, 0.1f, 2.0f);
	const auto cells = navmesh_cells(*in.navmesh);
	const auto num_cells = cells.layout.total;

	constexpr auto unreachable = augs::grid_distance_unreachable_v;

	const auto from_resistance = cells.distances_from(cells.find_all(in.resistance_spawns));
	const auto from_metropolis = cells.distances_from(cells.find_all(in.metropolis_spawns));

	/*
		The free cells reachable from any spawn are the playable area - the counts follow its size,
		tuned so that de_silo, de_cyberaqua and fy_minilab get about as many as their authors placed
		(1.6 patrols per million square pixels of free cells at most, a few pushes per side).
		The spacing usually stops the sampling earlier on a small or a twisted map.
	*/

	auto free_area = 0.0;

	for (std::size_t c = 0; c < num_cells; ++c) {
		if (cells.free(c) && (from_resistance[c] != unreachable || from_metropolis[c] != unreachable)) {
			const auto cell_size = static_cast<double>(in.navmesh->islands[cells.island_of(c)].cell_size);
			free_area += cell_size * cell_size;
		}
	}

	if (free_area <= 0.0) {
		return output;
	}

	const auto area_mpx = static_cast<real32>(free_area / 1e6);

	auto rounded = [](const real32 v) {
		return static_cast<int>(v + 0.5f);
	};

	const auto patrol_cap = std::clamp(rounded(area_mpx * 1.6f * density), 4, 60);
	const auto resistance_push_cap = std::clamp(rounded((3.0f + area_mpx / 20.0f) * density), 1, 8);
	const auto metropolis_push_cap = std::clamp(rounded((2.0f + area_mpx / 30.0f) * density), 1, 6);

	/*
		The nearest bombsite of every cell, by path.
	*/

	std::vector<uint32_t> from_site(num_cells, unreachable);
	std::vector<marker_letter_type> nearest_site(num_cells, marker_letter_type::A);

	for (const auto& site : in.bombsites) {
		auto site_cells = cells.find_free_within(site.first);

		if (site_cells.empty()) {
			if (const auto center = cells.find(site.first.get_center())) {
				site_cells.push_back(*center);
			}
		}

		if (!site_cells.empty()) {
			const auto site_distances = cells.distances_from(site_cells);

			for (std::size_t c = 0; c < num_cells; ++c) {
				if (site_distances[c] < from_site[c]) {
					from_site[c] = site_distances[c];
					nearest_site[c] = site.second;
				}
			}
		}
	}

	/*
		The spacing in path cost, from the most common cell size.
	*/

	const auto cell_size = [&]() {
		for (const auto& island : in.navmesh->islands) {
			if (island.cell_size > 0) {
				return static_cast<real32>(island.cell_size);
			}
		}

		return 128.0f;
	}();

	const auto spacing = static_cast<uint32_t>(base_spacing_px / density / cell_size * augs::grid_straight_step_cost_v);
	const auto push_spacing = static_cast<uint32_t>(static_cast<real32>(spacing) * push_spacing_mult);

	/*
		Weighted Poisson disk sampling: the candidates in a random order where those with a higher weight tend to come first,
		each taken if it is no closer than the spacing by path to anything taken so far or to the seeds (the author's waypoints).
		Weights prefer the main routes (see calc_route_weight), so waypoints rarely end up in dead ends at the edges of the map.
		Integers only - the same order on every machine.
	*/

	auto sample = [&](
		const std::vector<std::size_t>& candidates,
		const std::vector<uint32_t>& weights,
		const std::vector<std::size_t>& seeds,
		const int cap,
		const uint32_t min_spacing
	) {
		std::vector<std::size_t> picks;

		if (candidates.empty() || cap <= 0) {
			return picks;
		}

		std::vector<std::pair<uint64_t, std::size_t>> order;
		order.reserve(candidates.size());

		for (std::size_t i = 0; i < candidates.size(); ++i) {
			const auto key = static_cast<uint64_t>(rng.randval(0u, 1u << 20)) * weights[i];
			order.emplace_back(key, candidates[i]);
		}

		std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) {
			return a.first != b.first ? a.first > b.first : a.second < b.second;
		});

		/*
			Only whether a candidate is within the spacing matters - farther distances are never computed.
		*/

		std::vector<uint32_t> nearest(num_cells, unreachable);
		cells.lower(seeds, nearest, min_spacing);

		for (const auto& entry : order) {
			if (static_cast<int>(picks.size()) >= cap) {
				break;
			}

			const auto candidate = entry.second;

			if (nearest[candidate] < min_spacing) {
				continue;
			}

			picks.push_back(candidate);
			cells.lower({ candidate }, nearest, min_spacing);
		}

		return picks;
	};

	/*
		How close a cell lies to a main route - from a spawn to a bombsite, or between the spawns:
		the detour of passing through it over the shortest such path, 1000 on a route, less the more it costs to come by.
	*/

	auto shortest_through = [&](const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		auto best = unreachable;

		for (std::size_t c = 0; c < num_cells; ++c) {
			if (a[c] != unreachable && b[c] != unreachable) {
				best = std::min(best, a[c] + b[c]);
			}
		}

		return best;
	};

	const auto route_res_site = shortest_through(from_resistance, from_site);
	const auto route_met_site = shortest_through(from_metropolis, from_site);
	const auto route_res_met = shortest_through(from_resistance, from_metropolis);

	const auto detour_scale = static_cast<uint64_t>(spacing);

	auto calc_route_weight = [&](const std::size_t c) -> uint32_t {
		auto detour = uint64_t(unreachable);

		auto consider = [&](const std::vector<uint32_t>& a, const std::vector<uint32_t>& b, const uint32_t shortest) {
			if (shortest != unreachable && a[c] != unreachable && b[c] != unreachable) {
				detour = std::min(detour, static_cast<uint64_t>(a[c]) + b[c] - shortest);
			}
		};

		consider(from_resistance, from_site, route_res_site);
		consider(from_metropolis, from_site, route_met_site);
		consider(from_resistance, from_metropolis, route_res_met);

		if (detour == unreachable) {
			return 1000;
		}

		return static_cast<uint32_t>(50 + 950 * detour_scale / (detour_scale + detour));
	};

	auto existing_cells = [&](const point_marker_type type, const faction_type faction) {
		std::vector<vec2> positions;

		for (const auto& w : in.existing) {
			const bool same_side = faction == faction_type::ANY || w.faction == faction_type::ANY || w.faction == faction;

			if (w.type == type && same_side) {
				positions.push_back(w.pos);
			}
		}

		return cells.find_all(positions);
	};

	const bool has_both_sides = !in.gun_game && !in.metropolis_spawns.empty() && !in.resistance_spawns.empty();
	const bool has_sites = !in.bombsites.empty();

	/*
		Patrols - shared by both teams, between the spawns (in gun game anywhere reachable),
		each guarding the bombsite nearest to it.
	*/

	std::vector<std::size_t> patrol_outputs;

	{
		std::vector<std::size_t> candidates;

		for (std::size_t c = 0; c < num_cells; ++c) {
			if (!cells.free(c)) {
				continue;
			}

			const auto own = from_resistance[c];
			const auto other = from_metropolis[c];

			/*
				Around a bombsite always - even one right next to a spawn.
			*/

			const bool near_site = from_site[c] != unreachable && from_site[c] <= spacing * 2;

			if (has_both_sides && !near_site) {
				if (own == unreachable || other == unreachable) {
					continue;
				}

				const auto front = ::calc_front_permille(own, other);

				if (front < 150 || front > 850) {
					continue;
				}
			}
			else if (own == unreachable && other == unreachable) {
				continue;
			}

			candidates.push_back(c);
		}

		/*
			Patrols guard the bombsites - like on the official maps, most stand around them, the rest along the routes.
		*/

		const auto site_scale = static_cast<uint64_t>(spacing) * 2;

		auto calc_site_weight = [&](const std::size_t c) -> uint64_t {
			if (from_site[c] == unreachable) {
				return 1000;
			}

			return 1000 * site_scale / (site_scale + from_site[c]);
		};

		std::vector<uint32_t> weights;

		for (const auto c : candidates) {
			if (in.gun_game || !has_sites) {
				weights.push_back(in.gun_game ? 1000u : calc_route_weight(c));
			}
			else {
				weights.push_back(static_cast<uint32_t>((calc_site_weight(c) * 7 + calc_route_weight(c) * 3) / 10));
			}
		}

		auto picks = sample(candidates, weights, existing_cells(point_marker_type::BOT_WAYPOINT_PATROL, faction_type::ANY), patrol_cap, spacing);

		/*
			Bots patrol the bombsite they are assigned to, so every bombsite needs its own patrols -
			one the sampling left without any gets the free cells nearest to it, spaced among themselves.
		*/

		if (!in.gun_game) {
			for (const auto& site : in.bombsites) {
				const auto letter = site.second;

				auto count_of_letter = [&]() {
					auto n = 0;

					for (const auto pick : picks) {
						n += nearest_site[pick] == letter ? 1 : 0;
					}

					for (const auto& w : in.existing) {
						n += w.type == point_marker_type::BOT_WAYPOINT_PATROL && w.letter == letter ? 1 : 0;
					}

					return n;
				};

				constexpr int min_patrols_per_site = 2;

				if (count_of_letter() >= min_patrols_per_site) {
					continue;
				}

				std::vector<std::pair<uint32_t, std::size_t>> nearest_cells;

				for (std::size_t c = 0; c < num_cells; ++c) {
					if (cells.free(c) && nearest_site[c] == letter && from_site[c] != unreachable) {
						nearest_cells.emplace_back(from_site[c], c);
					}
				}

				std::sort(nearest_cells.begin(), nearest_cells.end());

				std::vector<std::size_t> added;

				for (const auto& entry : nearest_cells) {
					if (count_of_letter() + static_cast<int>(added.size()) >= min_patrols_per_site) {
						break;
					}

					const auto too_close = std::any_of(added.begin(), added.end(), [&](const auto other) {
						return (cells.center(other) - cells.center(entry.second)).length() < base_spacing_px / 3;
					});

					if (!too_close) {
						added.push_back(entry.second);
					}
				}

				picks.insert(picks.end(), added.begin(), added.end());
			}
		}

		for (const auto cell : picks) {
			random_waypoint w;
			w.pos = cells.center(cell);
			w.type = point_marker_type::BOT_WAYPOINT_PATROL;
			w.faction = faction_type::ANY;
			w.letter = !in.gun_game && has_sites ? nearest_site[cell] : marker_letter_type::A;
			w.camp = true;

			patrol_outputs.push_back(output.size());
			output.push_back(w);
		}
	}

	/*
		Pushes - a little past the middle towards the enemy: the attackers' towards the bombsites,
		the defenders' towards the attackers' spawn.
	*/

	if (has_both_sides) {
		auto make_pushes = [&](
			const faction_type faction,
			const std::vector<uint32_t>& own_side,
			const std::vector<uint32_t>& towards,
			const uint32_t min_front,
			const uint32_t max_front,
			const int cap
		) {
			std::vector<std::size_t> candidates;

			for (std::size_t c = 0; c < num_cells; ++c) {
				if (!cells.free(c) || own_side[c] == unreachable || towards[c] == unreachable) {
					continue;
				}

				const auto front = ::calc_front_permille(own_side[c], towards[c]);

				if (front >= min_front && front <= max_front) {
					candidates.push_back(c);
				}
			}

			std::vector<uint32_t> weights;

			for (const auto c : candidates) {
				weights.push_back(calc_route_weight(c));
			}

			for (const auto cell : sample(candidates, weights, existing_cells(point_marker_type::BOT_WAYPOINT_PUSH, faction), cap, push_spacing)) {
				random_waypoint w;
				w.pos = cells.center(cell);
				w.rotation = cells.calc_facing_towards(cell, towards);
				w.type = point_marker_type::BOT_WAYPOINT_PUSH;
				w.faction = faction;

				output.push_back(w);
			}
		};

		if (has_sites) {
			make_pushes(faction_type::RESISTANCE, from_resistance, from_site, 500, 850, resistance_push_cap);
		}
		else {
			make_pushes(faction_type::RESISTANCE, from_resistance, from_metropolis, 500, 750, resistance_push_cap);
		}

		make_pushes(faction_type::METROPOLIS, from_metropolis, from_resistance, 350, 600, metropolis_push_cap);
	}

	/*
		Rating the patrols with rays, at most max_total_rays of them in total.
	*/

	if (in.free_distance && !patrol_outputs.empty()) {
		const auto rays_per_patrol = std::clamp(max_total_rays / static_cast<int>(patrol_outputs.size()), min_rays_per_patrol, max_rays_per_patrol);

		for (const auto index : patrol_outputs) {
			auto& w = output[index];
			const auto rating = ::rate_patrol_spot(w.pos, rays_per_patrol, in.free_distance);

			w.rotation = rating.facing;

			if (rating.quality >= 0.45f) {
				w.camp_secs_min = 6.0f;
				w.camp_secs_max = 14.0f;
			}
			else if (rating.quality >= 0.18f) {
				w.camp_secs_min = 2.0f;
				w.camp_secs_max = 8.0f;
			}
			else {
				w.camp_secs_min = 1.0f;
				w.camp_secs_max = 3.0f;
			}

			const auto look_far_chance = std::clamp((rating.view_ahead - 1800.0f) / 1200.0f, 0.0f, 1.0f) * 0.45f;
			w.camp_look_far = rng.randval(0.0f, 1.0f) < look_far_chance;
		}
	}

	return output;
}
