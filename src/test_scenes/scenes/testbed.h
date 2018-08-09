#pragma once
#include "game/cosmos/cosmic_entropy.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/solvers/standard_solver.h"

class loaded_image_caches_map;
struct test_scene_mode_vars;
struct bomb_mode_vars;

namespace test_scenes {
	class testbed {
		void populate(const loaded_image_caches_map&, const logic_step) const;
	public:
		void populate_with_entities(const loaded_image_caches_map& caches, const logic_step_input input) {
			standard_solver()(
				input,
				[&](const logic_step step) { populate(caches, step); }, 
				[](auto) {},
				[](auto) {}
			);
		}

		void setup(test_scene_mode_vars&);
		void setup(bomb_mode_vars&);
	};
}