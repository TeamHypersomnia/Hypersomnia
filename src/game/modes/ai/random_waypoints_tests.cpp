#if !IS_PRODUCTION_BUILD
#if BUILD_UNIT_TESTS
#include <algorithm>
#include <cmath>
#include <Catch/single_include/catch2/catch.hpp>

#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/common_state/cosmos_navmesh.h"
#include "game/modes/ai/random_waypoints_generator.h"

/*
	The generator on hand-made navmeshes: open fields of 128 px cells, no walls for the rays.
*/

static constexpr uint32_t test_cell_size = 128;

static cosmos_navmesh_island make_open_island(const vec2i lt, const vec2u size_in_cells) {
	cosmos_navmesh_island island;
	island.cell_size = test_cell_size;
	island.bound = ltrbi(lt, lt + vec2i(size_in_cells * test_cell_size));
	island.resize_to_bounds();
	return island;
}

static vec2 cell_center(const cosmos_navmesh_island& island, const vec2u cell) {
	return vec2(island.bound.lt()) + vec2(cell) * static_cast<real32>(test_cell_size) + vec2::square(test_cell_size / 2.0f);
}

static random_waypoints_input make_input(const cosmos_navmesh& navmesh, const uint32_t seed) {
	random_waypoints_input in;
	in.navmesh = &navmesh;
	in.seed = seed;
	in.free_distance = [](const vec2 from, const vec2 to) { return (to - from).length(); };

	/*
		The spacing of 1100 px between patrols of a team, as the official maps have.
	*/

	in.density = 0.5f;
	return in;
}

static std::vector<vec2> positions_of(
	const std::vector<random_waypoint>& waypoints,
	const point_marker_type type,
	const faction_type faction = faction_type::COUNT
) {
	std::vector<vec2> out;

	for (const auto& w : waypoints) {
		if (w.type == type && (faction == faction_type::COUNT || w.faction == faction)) {
			out.push_back(w.pos);
		}
	}

	return out;
}

TEST_CASE("RandomWaypoints OpenField") {
	cosmos_navmesh navmesh;
	navmesh.islands.push_back(::make_open_island(vec2i::zero, vec2u(60, 30)));
	const auto& island = navmesh.islands[0];

	auto in = ::make_input(navmesh, 7);
	in.resistance_spawns = { ::cell_center(island, vec2u(2, 15)) };
	in.metropolis_spawns = { ::cell_center(island, vec2u(57, 15)) };
	in.bombsites = { { ltrb::center_and_size(::cell_center(island, vec2u(40, 8)), vec2::square(512)), marker_letter_type::A } };

	const auto first = ::generate_random_waypoints(in);

	SECTION("The same seed gives the same waypoints") {
		const auto again = ::generate_random_waypoints(in);

		REQUIRE(first.size() == again.size());

		for (std::size_t i = 0; i < first.size(); ++i) {
			REQUIRE(first[i].pos == again[i].pos);
			REQUIRE(first[i].type == again[i].type);
			REQUIRE(first[i].camp_look_far == again[i].camp_look_far);
		}
	}

	SECTION("Another seed gives other waypoints") {
		in.seed = 8;
		const auto other = ::generate_random_waypoints(in);

		REQUIRE(positions_of(other, point_marker_type::BOT_WAYPOINT_PATROL) != positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL));
	}

	SECTION("Patrols and pushes of both sides") {
		REQUIRE(positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL).size() >= 4);

		auto pushes_of = [&](const faction_type faction) {
			return std::count_if(first.begin(), first.end(), [&](const auto& w) {
				return w.type == point_marker_type::BOT_WAYPOINT_PUSH && w.faction == faction;
			});
		};

		REQUIRE(pushes_of(faction_type::RESISTANCE) >= 1);
		REQUIRE(pushes_of(faction_type::METROPOLIS) >= 1);
	}

	SECTION("Patrols of a team keep the spacing - on an open field the straight distance is about the path one") {
		for (const auto faction : { faction_type::METROPOLIS, faction_type::RESISTANCE }) {
			const auto patrols = positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL, faction);

			for (std::size_t a = 0; a < patrols.size(); ++a) {
				for (std::size_t b = a + 1; b < patrols.size(); ++b) {
					REQUIRE((patrols[a] - patrols[b]).length() >= 1100.0f * 0.7f);
				}
			}
		}
	}

	SECTION("Each team has its own patrols - none for both") {
		REQUIRE(positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL, faction_type::METROPOLIS).size() >= 2);
		REQUIRE(positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL, faction_type::RESISTANCE).size() >= 2);
		REQUIRE(positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL, faction_type::ANY).empty());
	}

	SECTION("The attackers' patrols stay around the bombsite") {
		const auto site_center = ::cell_center(island, vec2u(40, 8));

		for (const auto& w : positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL, faction_type::RESISTANCE)) {
			REQUIRE((w - site_center).length() <= 1100.0f * 1.5f + 512.0f);
		}
	}

	SECTION("Density 1 has about twice the patrols of density 0.5") {
		in.density = 1.0f;
		const auto denser = positions_of(::generate_random_waypoints(in), point_marker_type::BOT_WAYPOINT_PATROL);
		REQUIRE(denser.size() > positions_of(first, point_marker_type::BOT_WAYPOINT_PATROL).size() * 3 / 2);
	}

	SECTION("The author's patrols are complemented, never covered again") {
		const auto authors = ::cell_center(island, vec2u(30, 15));

		random_waypoint existing;
		existing.pos = authors;
		existing.type = point_marker_type::BOT_WAYPOINT_PATROL;
		in.existing = { existing };

		for (const auto& w : positions_of(::generate_random_waypoints(in), point_marker_type::BOT_WAYPOINT_PATROL)) {
			REQUIRE((w - authors).length() >= 1100.0f * 0.7f);
		}
	}
}

TEST_CASE("RandomWaypoints DefendersNeverFaceAwayFromTheAttackers") {
	/*
		Every spot sees farthest towards the defenders' spawn (a long corridor to it) - the longest view alone
		would turn the defenders' patrols with their backs to where the attackers come from (de_silo's "Random patrol 28").
	*/

	cosmos_navmesh navmesh;
	navmesh.islands.push_back(::make_open_island(vec2i::zero, vec2u(60, 30)));
	const auto& island = navmesh.islands[0];

	auto in = ::make_input(navmesh, 11);
	in.resistance_spawns = { ::cell_center(island, vec2u(2, 15)) };
	in.metropolis_spawns = { ::cell_center(island, vec2u(57, 15)) };
	in.bombsites = { { ltrb::center_and_size(::cell_center(island, vec2u(30, 15)), vec2::square(512)), marker_letter_type::A } };

	in.free_distance = [](const vec2 from, const vec2 to) {
		const auto dir = to - from;
		const bool towards_defenders = dir.x > 0.0f && std::abs(dir.y) < dir.x * 0.3f;
		return towards_defenders ? dir.length() : std::min(dir.length(), 2500.0f);
	};

	for (uint32_t seed = 1; seed <= 20; ++seed) {
		in.seed = seed;

		for (const auto& w : ::generate_random_waypoints(in)) {
			if (w.type == point_marker_type::BOT_WAYPOINT_PATROL && w.faction == faction_type::METROPOLIS) {
				/*
					On an open field the attackers come straight from their spawn.
				*/

				const auto attackers_from = in.resistance_spawns[0] - w.pos;
				const auto off = vec2::from_degrees(w.rotation).degrees_between(attackers_from);
				REQUIRE(off <= 90.0f);
			}
		}
	}
}

TEST_CASE("RandomWaypoints EveryBombsiteGetsPatrols") {
	cosmos_navmesh navmesh;
	navmesh.islands.push_back(::make_open_island(vec2i::zero, vec2u(60, 30)));
	const auto& island = navmesh.islands[0];

	auto in = ::make_input(navmesh, 3);
	in.resistance_spawns = { ::cell_center(island, vec2u(2, 15)) };
	in.metropolis_spawns = { ::cell_center(island, vec2u(57, 15)) };

	/*
		B right next to the defenders' spawn - outside the band between the spawns where patrols usually go.
	*/

	in.bombsites = {
		{ ltrb::center_and_size(::cell_center(island, vec2u(30, 15)), vec2::square(512)), marker_letter_type::A },
		{ ltrb::center_and_size(::cell_center(island, vec2u(58, 15)), vec2::square(512)), marker_letter_type::B }
	};

	for (uint32_t seed = 1; seed <= 20; ++seed) {
		in.seed = seed;

		const auto generated = ::generate_random_waypoints(in);

		auto patrols_of = [&](const marker_letter_type letter, const faction_type faction) {
			return std::count_if(generated.begin(), generated.end(), [&](const auto& w) {
				return w.type == point_marker_type::BOT_WAYPOINT_PATROL && w.letter == letter && w.faction == faction;
			});
		};

		for (const auto faction : { faction_type::METROPOLIS, faction_type::RESISTANCE }) {
			REQUIRE(patrols_of(marker_letter_type::A, faction) >= 2);
			REQUIRE(patrols_of(marker_letter_type::B, faction) >= 2);
		}
	}
}

TEST_CASE("RandomWaypoints AcrossPortals") {
	/*
		Two islands with nothing between them but a portal - each team spawns on another one.
	*/

	cosmos_navmesh navmesh;
	navmesh.islands.push_back(::make_open_island(vec2i::zero, vec2u(30, 20)));
	navmesh.islands.push_back(::make_open_island(vec2i(10000, 0), vec2u(30, 20)));

	auto& first = navmesh.islands[0];
	auto& second = navmesh.islands[1];

	auto in = ::make_input(navmesh, 5);
	in.resistance_spawns = { ::cell_center(first, vec2u(2, 10)) };
	in.metropolis_spawns = { ::cell_center(second, vec2u(27, 10)) };
	in.bombsites = { { ltrb::center_and_size(::cell_center(second, vec2u(10, 10)), vec2::square(512)), marker_letter_type::A } };

	auto count_on = [&](const std::vector<random_waypoint>& generated, const cosmos_navmesh_island& island) {
		return std::count_if(generated.begin(), generated.end(), [&](const auto& w) {
			return ltrb(island.bound).hover(w.pos);
		});
	};

	SECTION("Without the portal no push can be placed - the sides never meet") {
		const auto generated = ::generate_random_waypoints(in);
		REQUIRE(positions_of(generated, point_marker_type::BOT_WAYPOINT_PUSH).empty());
	}

	SECTION("With portals both ways (as maps have them) both islands get waypoints") {
		auto connect = [&](cosmos_navmesh_island& from, const vec2u in_cell, const vec2u out_cell, const int out_island) {
			from.occupied[from.cell_index(in_cell)] = static_cast<uint8_t>(2 + from.portals.size());

			navmesh_portal portal;
			portal.in_cell_pos = in_cell;
			portal.out_cell_pos = out_cell;
			portal.in_world_pos = ::cell_center(from, in_cell);
			portal.out_island_index = out_island;
			from.portals.push_back(portal);
		};

		connect(first, vec2u(28, 10), vec2u(2, 10), 1);
		connect(second, vec2u(1, 10), vec2u(27, 10), 0);

		const auto generated = ::generate_random_waypoints(in);

		REQUIRE(count_on(generated, first) >= 1);
		REQUIRE(count_on(generated, second) >= 1);
		REQUIRE(!positions_of(generated, point_marker_type::BOT_WAYPOINT_PUSH).empty());
	}
}
#endif
#endif
