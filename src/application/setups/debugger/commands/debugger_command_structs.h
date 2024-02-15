#pragma once
#include "augs/misc/date_time.h"
#include "augs/templates/snapshotted_player_step_type.h"

struct debugger_command_common {
	// GEN INTROSPECTOR struct debugger_command_common
	std::time_t timestamp = {};
	augs::snapshotted_player_step_type when_happened = 0;
	bool has_parent = false;
	// END GEN INTROSPECTOR

	void reset_timestamp() {
		timestamp = augs::date_time();
	}
};
