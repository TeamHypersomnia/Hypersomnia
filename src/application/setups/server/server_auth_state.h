#pragma once
#include "augs/misc/constant_size_string.h"
#include "application/setups/server/rcon_level.h"

/*
	A random string the server makes for every client - the identity it requests its Steam ticket for,
	so that a ticket given to one server can not be simply replayed on another - see steam_request_auth_ticket.
	A server could still pass on the challenge of another server it is connected to,
	so the tickets are only sent to the official servers.
*/

using auth_challenge_type = augs::constant_size_string<32>;

/*
	Sent to a client once it connects with the challenge,
	and again once its authentication changes its rcon level - e.g. by its admin profile.
*/

struct server_auth_state {
	auth_challenge_type challenge;
	rcon_level_type rcon_level = rcon_level_type::DENIED;
	bool requires_authentication = false;
};
