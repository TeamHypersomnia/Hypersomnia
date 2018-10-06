#pragma once
#include "augs/misc/time_utils.h"
#include "application/setups/editor/player/editor_player_step_type.h"

struct editor_command_common {
	// GEN INTROSPECTOR struct editor_command_common
	std::time_t timestamp = {};
	editor_player_step_type when_happened = 0;
	bool has_parent = false;
	// END GEN INTROSPECTOR

	void reset_timestamp() {
		timestamp = augs::date_time();
	}
};
