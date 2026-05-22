#pragma once
#include <string>

#include "augs/misc/secure_hash.h"
#include "augs/network/network_types.h"

/*
	Header saved alongside the serialized solvable state so that a recovered server
	can verify it loads into the right arena and mode. The ranked account ids are part
	of the serialized mode state itself, so they need no separate bookkeeping here.
*/

struct server_recovery_metadata {
	std::string version;
	arena_identifier arena_name;
	game_mode_name_type game_mode;
	augs::secure_hash_type arena_hash = augs::secure_hash_type();
};
