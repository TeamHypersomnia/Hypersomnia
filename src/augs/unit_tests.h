#pragma once
#include <string>

struct unit_tests_settings;

namespace augs {
	void run_unit_tests(
		const int argc,
		const char* const * const argv,
		const unit_tests_settings&
	);
}