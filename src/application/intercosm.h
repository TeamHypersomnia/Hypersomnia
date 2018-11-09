#pragma once
#include "augs/enums/callback_result.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
#include "view/viewables/all_viewables_defs.h"
#include "hypersomnia_version.h"

namespace sol {
	class state;
}

struct intercosm_path_op {
	sol::state& lua;
	augs::path_type path;
};

struct test_scene_settings;

struct test_mode_ruleset;
struct bomb_mode_ruleset;

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
		test_mode_ruleset&,
		bomb_mode_ruleset* = nullptr
	);
#endif

	void load(const intercosm_path_op);
	void save(const intercosm_path_op) const;

	void load_from_int(const augs::path_type&);
	void save_as_int(const augs::path_type&) const;

	void load_from_lua(const intercosm_path_op);
	void save_as_lua(const intercosm_path_op) const;

	void clear();
	void update_offsets_of(const assets::image_id&, changer_callback_result = changer_callback_result::REFRESH);

	void post_load_state_correction();
};

#if READWRITE_OVERLOAD_TRAITS_INCLUDED || LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& ar, const intercosm&);

	template <class Archive>
	void read_object_bytes(Archive& ar, intercosm&);

	template <class Archive>
	void write_object_lua(Archive&, const intercosm&);

	template <class Archive>
	void read_object_lua(const Archive&, intercosm&);
}