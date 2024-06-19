#pragma once
#include <string>
#include "augs/filesystem/path.h"
#include "augs/templates/exception_templates.h"

struct unit_tests_settings {
	// GEN INTROSPECTOR struct unit_tests_settings
	bool run = false;
	bool log_successful = false;
	bool break_on_failure = false;

	augs::path_type redirect_log_to_path = "";
	// END GEN INTROSPECTOR

	bool operator==(const unit_tests_settings& b) const = default;
};

namespace augs {
	struct unit_test_session_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;

		std::string cout_content;
		std::string cerr_content;
		std::string clog_content;
	};

	void run_unit_tests(const unit_tests_settings&);
}