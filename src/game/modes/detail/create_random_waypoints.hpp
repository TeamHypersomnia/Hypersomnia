#pragma once
#include "game/cosmos/allocate_new_entity_access.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/just_create_entity.h"
#include "game/components/marker_component.h"
#include "game/enums/filters.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/modes/ai/random_waypoints_generator.h"

/*
	The bots' waypoints of this round, generated as point markers like the author's (see random_waypoints_generator.h) -
	right after the clean round state was restored, so the previous round's are gone already.
	Every client generates the same ones: from the round's seed, the navmesh and the map's walls.
	Uses allocate_new_entity_access: called from arena_mode::setup_round, before anything holds handles.
*/

/*
	Everything the generator needs from a cosmos: the navmesh, spawns, bombsites, the waypoints already there and rays against its walls.
	The rays refer to the cosmos - use the input only while it lives. Shared by the game and the editor's preview.
*/

inline random_waypoints_input gather_random_waypoints_input(const cosmos& cosm, const bool gun_game) {
	const auto& common = cosm.get_common_significant();

	random_waypoints_input input;
	input.navmesh = &common.navmesh;
	input.gun_game = gun_game;
	input.density = common.bot_waypoints.density;

	std::vector<vec2> ffa_spawns;

	cosm.for_each_having<invariants::point_marker>([&](const auto& typed_handle) {
		const auto type = typed_handle.template get<invariants::point_marker>().type;
		const auto transform = typed_handle.get_logic_transform();

		if (type == point_marker_type::TEAM_SPAWN) {
			const auto faction = typed_handle.get_official_faction();

			if (faction == faction_type::METROPOLIS) {
				input.metropolis_spawns.push_back(transform.pos);
			}
			else {
				input.resistance_spawns.push_back(transform.pos);
			}
		}
		else if (type == point_marker_type::FFA_SPAWN) {
			ffa_spawns.push_back(transform.pos);
		}
		else if (::is_waypoint(type)) {
			const auto& marker = typed_handle.template get<components::marker>();

			random_waypoint w;
			w.pos = transform.pos;
			w.type = type;
			w.faction = marker.faction;
			w.letter = marker.letter;
			input.existing.push_back(w);
		}
	});

	if (gun_game) {
		/*
			Gun game has no sides - patrols anywhere reachable from any spawn.
		*/

		auto& all = input.resistance_spawns;
		all.insert(all.end(), input.metropolis_spawns.begin(), input.metropolis_spawns.end());
		all.insert(all.end(), ffa_spawns.begin(), ffa_spawns.end());
		input.metropolis_spawns.clear();
	}
	else {
		cosm.for_each_having<invariants::area_marker>([&](const auto& typed_handle) {
			if (::is_bombsite(typed_handle.template get<invariants::area_marker>().type)) {
				if (const auto area = typed_handle.find_aabb()) {
					input.bombsites.emplace_back(*area, typed_handle.template get<components::marker>().letter);
				}
			}
		});
	}

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	input.free_distance = [&physics, si](const vec2 from, const vec2 to) {
		const auto ray = physics.ray_cast_px(si, from, to, predefined_queries::line_of_sight());
		return ray.hit ? (ray.intersection - from).length() : (to - from).length();
	};

	return input;
}

inline void create_random_waypoints(
	allocate_new_entity_access access,
	cosmos& cosm,
	const bool gun_game,
	const uint32_t seed
) {
	const auto& common = cosm.get_common_significant();
	const auto mode = common.bot_waypoints.mode;

	auto input = ::gather_random_waypoints_input(cosm, gun_game);
	input.seed = seed;

	if (mode == bot_waypoints_mode::AUTOMATIC && !input.existing.empty()) {
		return;
	}

	if (mode == bot_waypoints_mode::RANDOM_EVERY_ROUND) {
		input.existing.clear();
	}

	const auto generated = ::generate_random_waypoints(input);

	if (generated.empty()) {
		return;
	}

	auto patrol_flavour = entity_flavour_id();
	auto push_flavour = entity_flavour_id();

	cosm.for_each_flavour_having<invariants::point_marker>([&](const auto& id, const auto& flavour) {
		const auto type = flavour.template get<invariants::point_marker>().type;

		if (type == point_marker_type::BOT_WAYPOINT_PATROL && !patrol_flavour.is_set()) {
			patrol_flavour = entity_flavour_id(id);
		}
		else if (type == point_marker_type::BOT_WAYPOINT_PUSH && !push_flavour.is_set()) {
			push_flavour = entity_flavour_id(id);
		}
	});

	for (const auto& w : generated) {
		const auto flavour = w.type == point_marker_type::BOT_WAYPOINT_PUSH ? push_flavour : patrol_flavour;

		if (!flavour.is_set()) {
			continue;
		}

		just_create_entity(
			access,
			cosm,
			flavour,
			[&](const entity_handle new_waypoint) {
				new_waypoint.set_logic_transform(transformr(w.pos, w.rotation));

				if (const auto marker = new_waypoint.find<components::marker>()) {
					marker->faction = w.faction;
					marker->letter = w.letter;
					marker->camp = w.camp;
					marker->camp_look_far = w.camp_look_far;
					marker->camp_secs_min = w.camp_secs_min;
					marker->camp_secs_max = w.camp_secs_max;
				}
			},
			[](entity_handle) {}
		);
	}
}
