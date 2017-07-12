#pragma once
#include <string>

namespace augs {
	void run_unit_tests(
		const int argc,
		const char* const * const argv,
		const bool show_successful,
		const bool break_on_failure,
		const std::string& output_log_path
	);
}