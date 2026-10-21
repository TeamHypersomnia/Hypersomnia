#pragma once
#include "augs/filesystem/path_declaration.h"
#include "game/cosmos/step_declaration.h"

class loaded_image_caches_map;
struct test_mode_ruleset;
struct arena_mode_ruleset;


namespace test_scenes {
	/*
		Writes the weapon stats reported while the testbed was populated - see --export-weapon-stats.
	*/

	void save_reported_weapons(const augs::path_type& dir);

	class testbed {
		void populate(const loaded_image_caches_map&, const logic_step) const;
	public:
		void populate_with_entities(const loaded_image_caches_map& caches, const logic_step_input input);
	};
}