#pragma once
#include <cstddef>
#include <utility>
#include <variant>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/make_physics_path_hints.h"
#include "game/detail/pathfinding/pathfinding.h"
#include "game/modes/test_mode.h"
#include "game/modes/quick_test_spawn_order.h"

using bot_test_path_ends = std::vector<std::pair<vec2, vec2>>;

inline bot_test_path_ends make_bot_test_path_ends(
	const cosmos& cosm,
	const test_mode_ruleset& rules,
	const faction_type host_faction
) {
	auto bot_spawns = std::vector<entity_id>();

	::for_each_quick_test_spawn_in_order(cosm, rules.quick_test_spawns, host_faction, [&](const playtest_spawn& spawn_def, const faction_type, const bool for_host) {
		if (!for_host) {
			bot_spawns.push_back(spawn_def.spawn);
		}
	});

	const auto assigned_ends = ::assign_bot_test_navigation_ends(cosm, rules.bot_test_navigation_ends, bot_spawns);

	auto path_ends = bot_test_path_ends();

	for (std::size_t i = 0; i < bot_spawns.size(); ++i) {
		if (const auto& end_def = assigned_ends[i]; end_def.has_value()) {
			path_ends.emplace_back(
				cosm[bot_spawns[i]].get_logic_transform().pos,
				cosm[end_def->end].get_logic_transform().pos
			);
		}
	}

	return path_ends;
}

class editor_bot_test_paths {
	bot_test_path_ends computed_for;
	std::vector<pathfinding_path> paths;
	bool computed = false;

public:
	void invalidate() {
		computed = false;
	}

	template <class Ruleset>
	const std::vector<pathfinding_path>& get(const cosmos& cosm, const Ruleset& ruleset, const faction_type host_faction) {
		auto path_ends = bot_test_path_ends();

		if (const auto rules = std::get_if<test_mode_ruleset>(std::addressof(ruleset))) {
			path_ends = ::make_bot_test_path_ends(cosm, *rules, host_faction);
		}

		if (computed && path_ends == computed_for) {
			return paths;
		}

		computed = true;
		computed_for = path_ends;
		paths.clear();

		const auto& navmesh = cosm.get_common_significant().navmesh;
		const auto physics_hints = ::make_physics_path_hints(cosm);
		auto pathfinding = pathfinding_context();

		for (const auto& [from, to] : path_ends) {
			for (auto& path : ::find_path_across_islands_many_full(navmesh, from, to, std::addressof(physics_hints), std::addressof(pathfinding))) {
				paths.push_back(std::move(path));
			}
		}

		return paths;
	}
};

template <class OnScreen, class DrawCell, class DrawArrow>
void draw_bot_test_paths(
	const cosmos_navmesh& navmesh,
	const std::vector<pathfinding_path>& paths,
	OnScreen on_screen,
	DrawCell draw_cell,
	DrawArrow draw_arrow
) {
	for (const auto& path : paths) {
		if (path.island_index >= navmesh.islands.size()) {
			continue;
		}

		const auto& island = navmesh.islands[path.island_index];
		const auto cell_size = static_cast<float>(island.cell_size);

		if (cell_size <= 0) {
			continue;
		}

		for (std::size_t i = 0; i < path.nodes.size(); ++i) {
			const auto& node = path.nodes[i];
			const auto world_lt = vec2(island.bound.lt()) + vec2(node.cell_xy) * cell_size;
			const auto screen_lt = on_screen(world_lt);
			const auto screen_rb = on_screen(world_lt + vec2::square(cell_size));

			draw_cell(ltrb(screen_lt, screen_rb - screen_lt));

			if (i + 1 < path.nodes.size()) {
				const auto direction = vec2i(path.nodes[i + 1].cell_xy) - vec2i(node.cell_xy);
				const auto center_screen = on_screen(world_lt + vec2::square(cell_size / 2));

				draw_arrow(center_screen, vec2(direction).degrees());
			}
		}
	}
}
