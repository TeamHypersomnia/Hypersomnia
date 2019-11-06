#pragma once
#include "augs/enums/callback_result.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
#include "view/viewables/all_viewables_defs.h"
#include "application/arena/intercosm_paths.h"

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

void post_load_state_correction(
	cosmos_common_significant&,
	const all_viewables_defs&
);

struct intercosm {
	// GEN INTROSPECTOR struct intercosm
	cosmos world;
	all_viewables_defs viewables;
	// END GEN INTROSPECTOR

	void make_test_scene(
		sol::state&, 
		test_scene_settings,
		test_mode_ruleset&,
		bomb_mode_ruleset* = nullptr
	);

	void load_from_bytes(const intercosm_paths&);
	void save_as_bytes(const intercosm_paths&) const;

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