#pragma once
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/enums/faction_type.h"
#include "game/enums/marker_type.h"

struct cosmos_navmesh;

/*
	Bots' push and patrol waypoints generated from the navmesh - every round from a seed all clients know,
	or once in the editor as a preview. Knows nothing of the cosmos: the input is the navmesh, spawns, bombsites,
	the author's waypoints to complement, and a ray query.

	Path distances over the navmesh cells from each team's spawns tell where the middle of the map is:
	pushes lie a little past it towards the enemy (the attackers' towards the bombsites), patrols between the spawns.
	Within each kind, weighted Poisson disk sampling spreads them out - never closer by path than the spacing
	to each other or to the author's waypoints, preferring the main routes between the spawns and the bombsites.
	Rays only rate the chosen patrols: how covered the spot is decides how long a bot camps there,
	how far it sees ahead - whether it looks far.
*/

struct random_waypoint {
	vec2 pos;
	real32 rotation = 0.0f;
	point_marker_type type = point_marker_type::BOT_WAYPOINT_PATROL;
	faction_type faction = faction_type::ANY;
	marker_letter_type letter = marker_letter_type::A;
	bool camp = false;
	bool camp_look_far = false;
	real32 camp_secs_min = 2.0f;
	real32 camp_secs_max = 10.0f;
};

struct random_waypoints_input {
	const cosmos_navmesh* navmesh = nullptr;

	std::vector<vec2> resistance_spawns;
	std::vector<vec2> metropolis_spawns;
	/* A bombsite's area - its center may well stand on an obstacle. */
	std::vector<std::pair<ltrb, marker_letter_type>> bombsites;

	/* The author's waypoints - the generated ones complement them, never covering the same spots. */
	std::vector<random_waypoint> existing;

	bool gun_game = false;
	real32 density = 1.0f;
	uint32_t seed = 0;

	/* How far a ray from `from` towards `to` gets before an obstacle - at most the distance between them. */
	std::function<real32(vec2 from, vec2 to)> free_distance;
};

std::vector<random_waypoint> generate_random_waypoints(const random_waypoints_input& in);

/*
	How good a spot is to camp at, from rays around it: a bot faces where it sees the farthest;
	the spot is good with its back and sides covered and an open view ahead (quality 0..1),
	open all around it is only for a quick look. Longer camps on better spots, look far with a long view ahead.
*/

struct patrol_spot_rating {
	real32 facing = 0.0f;
	real32 view_ahead = 0.0f;
	real32 quality = 0.0f;
};

patrol_spot_rating rate_patrol_spot(
	vec2 pos,
	int num_rays,
	const std::function<real32(vec2 from, vec2 to)>& free_distance
);
