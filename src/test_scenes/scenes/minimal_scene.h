#pragma once
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/standard_solver.h"

class cosmos;
struct loaded_game_image_caches;

namespace test_scenes {
	class minimal_scene {
		entity_id populate(const loaded_game_image_caches&, const logic_step) const;
	public:
		void populate(const loaded_game_image_caches&, cosmos_common_significant&) const;
		entity_id populate_with_entities(const loaded_game_image_caches& caches, const logic_step_input input) {
			entity_id controlled;

			standard_solver(
				input,
				[&](const logic_step step) { controlled = populate(caches, step); }, 
				[](auto) {},
				[](auto) {}
			);

			return controlled;
		}
	};
}