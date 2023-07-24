#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "hypersomnia_version.h"

/*
	Note: the official arenas do not specify either the game version or the autor's public key.
*/

struct editor_project_meta {
	// GEN INTROSPECTOR struct editor_project_meta
	game_version_identifier game_version;
	augs::constant_size_string<max_arena_name_length_v> name;
	augs::constant_size_string<arena_public_key_length_v> author_public_key;
	version_timestamp_string version_timestamp;
	// END GEN INTROSPECTOR

	bool operator==(const editor_project_meta&) const = default;
};
