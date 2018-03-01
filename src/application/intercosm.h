#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/standard_solver.h"

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

struct test_scene_settings;

struct intercosm {
	// GEN INTROSPECTOR struct intercosm
	hypersomnia_version version;

	cosmos world;
	all_viewables_defs viewables;
	
	entity_id local_test_subject;
	// END GEN INTROSPECTOR

#if BUILD_TEST_SCENES
	void make_test_scene(sol::state&, test_scene_settings);
#endif

	void load(const intercosm_path_op);
	void save(const intercosm_path_op) const;

	void load_as_int(const augs::path_type&);
	void save_as_int(const augs::path_type&) const;

	void load_as_lua(const intercosm_path_op);
	void save_as_lua(const intercosm_path_op) const;

	auto make_logic_step_input(const cosmic_entropy& entropy) {
		return logic_step_input{ world, entropy };	
	}

	template <class... Callbacks>
	void advance(
		const cosmic_entropy& entropy,
		Callbacks&&... callbacks
	) {
		standard_solver(
			make_logic_step_input(entropy),
			std::forward<Callbacks>(callbacks)...
		);
	}
};