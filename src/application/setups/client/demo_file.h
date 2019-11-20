#pragma once
#include <vector>
#include <map>
#include "application/setups/client/demo_file_meta.h"
#include "augs/templates/snapshotted_player_step_type.h"

struct demo_step;
using demo_step_num_type = augs::snapshotted_player_step_type;
using demo_step_map = std::map<demo_step_num_type, demo_step>;

struct demo_file {
	// GEN INTROSPECTOR struct demo_file
	demo_file_meta meta;
	demo_step_map steps;
	// END GEN INTROSPECTOR
};
