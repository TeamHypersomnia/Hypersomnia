#pragma once
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/cosmos/entity_id.h"

class cosmos;
class particles_simulation_system;
struct randomization;
struct explosions_settings;
struct continuous_rings_input;
struct particles_emission;

/*
	Spawns the portals' pixel particles (see continuous_rings_particles) - for making new portal effects,
	in developer builds (Settings > Portal effects: PARTICLES); the game draws the portals with a shader (portal_swirl.fsh).
*/

class portal_particles_spawner {
	/*
		The portals with pixel particles in view this frame - caches kept between the frames so as not to allocate.
	*/

	struct visible_portal {
		entity_id id;
		vec2 center;
		float radius = 0.f;
		const continuous_rings_input* rings = nullptr;
		float count = 0.f;
		int alive = 0;
	};

	std::vector<visible_portal> visible_portals;
	std::unordered_map<entity_id, std::size_t> visible_portal_indices;
	std::unordered_set<entity_id> portals_visible_last_frame;

public:
	void advance(
		const cosmos& cosm,
		randomization& rng,
		const ltrb queried_camera_aabb,
		const explosions_settings& settings,
		const particles_emission& emission,
		particles_simulation_system& particles,
		const float amount_mult,
		const float dt_secs
	);

	void clear();
};
