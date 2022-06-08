#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "hypersomnia_version.h"

struct editor_project_meta {
	// GEN INTROSPECTOR struct editor_project_about
	augs::constant_size_string<20> game_version;
	augs::constant_size_string<max_arena_name_length_v> name;
	augs::constant_size_string<arena_public_key_length_v> author_public_key;
	// END GEN INTROSPECTOR

	editor_project_meta() {
		game_version = hypersomnia_version().get_version_string();
	}
};
