#pragma once
#include "augs/misc/constant_size_string.h"
#include "application/setups/server/rcon_level.h"

/*
	Sent to a client once it is welcomed,
	and again once its authentication changes its rcon level - e.g. by its admin profile.
*/

struct server_auth_state {
	rcon_level_type rcon_level = rcon_level_type::DENIED;
	bool requires_authentication = false;
};
