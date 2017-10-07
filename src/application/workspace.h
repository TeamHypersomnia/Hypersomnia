#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_entropy.h"

#include "view/viewables/all_viewables_defs.h"

namespace sol {
	class state;
}

struct workspace {
	// GEN INTROSPECTOR struct workspace
	cosmos world;
	all_logical_assets logicals;
	all_viewables_defs viewables;
	
	entity_id locally_viewed;
	// END GEN INTROSPECTOR

#if BUILD_TEST_SCENES
	void make_test_scene(sol::state&, const bool minimal);
#endif

	void make_blank();

	template <class... Callbacks>
	void advance(
		const cosmic_entropy& entropy,
		Callbacks&&... callbacks
	) {
		world.advance(
			{ entropy, logicals },
			std::forward<Callbacks>(callbacks)...
		);
	}
};