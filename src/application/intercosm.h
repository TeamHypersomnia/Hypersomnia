#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmic_entropy.h"
#include "game/cosmos/standard_solver.h"

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

	intercosm() = default;

	intercosm(intercosm&&) = delete;
	intercosm& operator=(intercosm&&) = delete;

	intercosm(const intercosm&) = delete;
	intercosm& operator=(const intercosm&) = delete;

#if BUILD_TEST_SCENES
	void make_test_scene(sol::state&, test_scene_settings);
#endif

	void load(const intercosm_path_op);
	void save(const intercosm_path_op) const;

	void load_from_int(const augs::path_type&);
	void save_as_int(const augs::path_type&) const;

	void load_from_lua(const intercosm_path_op);
	void save_as_lua(const intercosm_path_op) const;

	void to_bytes(std::vector<std::byte>&) const;
	void from_bytes(const std::vector<std::byte>&);

	logic_step_input make_logic_step_input(const cosmic_entropy& entropy) {
		return { world, entropy };	
	}

	void clear();

	const_entity_handle get_viewed_character() const;

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

	void update_offsets_of(const assets::image_id&, changer_callback_result = changer_callback_result::REFRESH);
};