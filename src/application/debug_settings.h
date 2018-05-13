#pragma once
#include "augs/unit_tests.h"

enum class input_recording_type {
	// GEN INTROSPECTOR enum class input_recording_type
	DISABLED,
	LIVE,
	COUNT
	// END GEN INTROSPECTOR
};

struct debug_settings {
	// GEN INTROSPECTOR struct debug_settings
	unsigned determinism_test_cloned_cosmoi_count = 0;
	input_recording_type input_recording_mode = input_recording_type::DISABLED;
	bool measure_atlas_uploading = false;
	// END GEN INTROSPECTOR
};