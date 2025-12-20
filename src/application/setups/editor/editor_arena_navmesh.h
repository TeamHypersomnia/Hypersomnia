#pragma once
#include "augs/misc/secure_hash.h"
#include "game/common_state/cosmos_navmesh.h"

/*
    Binary file layout for arena navmesh saved next to the project .json:
    - secure hash of the arena json
    - cosmos_navmesh payload
*/
struct editor_arena_navmesh {
	// GEN INTROSPECTOR struct editor_arena_navmesh
	augs::secure_hash_type arena_hash;
	cosmos_navmesh navmesh;
	// END GEN INTROSPECTOR
};


