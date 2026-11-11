#pragma once
#include <cstdint>
#include "view/audiovisual_state/systems/particles_simulation_system.h"

inline void clear_particles_after_test_scene_rebuild(
	const std::uint32_t arena_rebuilds,
	std::uint32_t& arena_rebuilds_at_last_clear,
	particles_simulation_system& particles
) {
	if (arena_rebuilds != arena_rebuilds_at_last_clear) {
		arena_rebuilds_at_last_clear = arena_rebuilds;
		particles.clear();
	}
}
