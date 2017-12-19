#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_entropy.h"

#include "view/viewables/all_viewables_defs.h"
#include "hypersomnia_version.h"

namespace sol {
	class state;
}

struct intercosm_loading_error {
	std::string title;
	std::string message;
	std::string details;
};

struct intercosm_path_op {
	sol::state& lua;
	augs::path_type path;
};

struct intercosm {
	// GEN INTROSPECTOR struct intercosm
	hypersomnia_version version;

	cosmos world;
	cosmos_common_state common;
	all_logical_assets logicals;
	all_viewables_defs viewables;
	
	entity_id locally_viewed;
	// END GEN INTROSPECTOR

#if BUILD_TEST_SCENES
	void make_test_scene(sol::state&, const bool minimal);
#endif

	void make_blank();

	void open(const intercosm_path_op);
	void save(const intercosm_path_op) const;

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