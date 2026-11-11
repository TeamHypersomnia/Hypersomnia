#pragma once
#include <cstdint>
#include "view/audiovisual_state/systems/particles_simulation_system.h"

inline void clear_particles_left_by_previous_tutorial_stage(
	const std::uint32_t stages_built,
	std::uint32_t& stages_built_at_last_clear,
	particles_simulation_system& particles
) {
	if (stages_built != stages_built_at_last_clear) {
		stages_built_at_last_clear = stages_built;
		particles.clear();
	}
}
