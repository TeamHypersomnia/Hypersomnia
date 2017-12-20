#pragma once
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/standard_solver.h"

class cosmos;
struct all_logical_assets;

namespace test_scenes {
	class minimal_scene {
		void populate(const logic_step);
	public:
		void populate_world_with_entities(const logic_step_input input) {
			standard_solver(
				input,
				[&](const logic_step step) { populate(step); }, 
				[](auto) {},
				[](auto) {}
			);
		}
	};
}