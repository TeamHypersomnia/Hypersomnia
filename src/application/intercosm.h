#pragma once
#include "augs/enums/callback_result.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
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

struct test_scene_mode_vars;
struct bomb_mode_vars;

struct intercosm {
	// GEN INTROSPECTOR struct intercosm
	hypersomnia_version version;

	cosmos world;
	all_viewables_defs viewables;
	// END GEN INTROSPECTOR

#if BUILD_TEST_SCENES
	void make_test_scene(
		sol::state&, 
		test_scene_settings,
		test_scene_mode_vars&,
		bomb_mode_vars* = nullptr
	);
#endif

	void load(const intercosm_path_op);
	void save(const intercosm_path_op) const;

	void load_from_int(const augs::path_type&);
	void save_as_int(const augs::path_type&) const;

	void load_from_lua(const intercosm_path_op);
	void save_as_lua(const intercosm_path_op) const;

	void to_bytes(std::vector<std::byte>&) const;
	void from_bytes(const std::vector<std::byte>&);

	void clear();
	void update_offsets_of(const assets::image_id&, changer_callback_result = changer_callback_result::REFRESH);
};