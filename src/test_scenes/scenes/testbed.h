#pragma once
#include "game/cosmos/step_declaration.h"

class loaded_image_caches_map;
struct test_mode_ruleset;
struct arena_mode_ruleset;


namespace test_scenes {
	class testbed {
		void populate(const loaded_image_caches_map&, const logic_step) const;
	public:
		void populate_with_entities(const loaded_image_caches_map& caches, const logic_step_input input);
	};
}