#pragma once

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
	bool create_minimal_test_scene = false;
	bool show_developer_console = false;
	input_recording_type input_recording_mode = input_recording_type::DISABLED;
	// END GEN INTROSPECTOR
};

struct unit_tests_settings {
	// GEN INTROSPECTOR struct unit_tests_settings
	bool run = false;
	bool log_successful = false;
	bool break_on_failure = false;
	
	std::string output_log_path = "generated/logs/unit_tests.txt";
	// END GEN INTROSPECTOR
};