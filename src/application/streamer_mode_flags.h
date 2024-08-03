#pragma once
/*
	To not have to include it everywhere because we're using the checks in the same places
*/

#include "augs/misc/profanity_filter.h"

struct streamer_mode_flags_data {
	// GEN INTROSPECTOR struct streamer_mode_flags_data
	bool chat = true;
	bool chat_open = false;
	bool inworld_hud = true;
	bool scoreboard = true;
	bool spectator_ui = true;
	bool kill_notifications = true;
	bool death_summary = true;
	bool community_servers = true;
	bool map_catalogue = false;
	// END GEN INTROSPECTOR

	bool operator==(const streamer_mode_flags_data& b) const = default;
};

