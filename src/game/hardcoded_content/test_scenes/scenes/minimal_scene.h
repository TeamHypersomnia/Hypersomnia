#pragma once
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"

class cosmos;
class all_logical_assets;

namespace test_scenes {
	class minimal_scene {
		void populate(const logic_step);
	public:
		template <class T>
		void populate_world_with_entities(
			cosmos& cosm,
			const all_logical_assets& metas,
			const T post_solve
		) {
			cosm.advance(
				{ cosmic_entropy(), metas },
				[&](const logic_step step) { populate(step); }, 
				post_solve
			);
		}
	};
}