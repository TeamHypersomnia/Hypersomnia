#pragma once
#include "augs/filesystem/path_declaration.h"

struct float_consistency_test_settings {
	// GEN INTROSPECTOR struct float_consistency_test_settings
	int passes = 5000;
	augs::path_type report_filename;
	// END GEN INTROSPECTOR

	bool operator==(const float_consistency_test_settings& b) const = default;
};

void setup_float_flags();
bool perform_float_consistency_tests(const float_consistency_test_settings&);
void ensure_float_flags_hold();

