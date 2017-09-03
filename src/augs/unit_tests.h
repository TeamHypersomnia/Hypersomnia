#pragma once
#include <string>

struct unit_tests_settings {
	// GEN INTROSPECTOR struct unit_tests_settings
	bool run = false;
	bool log_successful = false;
	bool break_on_failure = false;

	std::string output_log_path = "generated/logs/unit_tests.txt";
	// END GEN INTROSPECTOR
};

namespace augs {
	void run_unit_tests(
		const int argc,
		const char* const * const argv,
		const unit_tests_settings&
	);
}