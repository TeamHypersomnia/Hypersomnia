#pragma once
#include "augs/string/typesafe_sscanf.h"
#include "augs/filesystem/path.h"

struct cmd_line_params {
	augs::path_type exe_path;
	augs::path_type editor_target;
	bool unit_tests_only = false;
	bool help_only = false;

	cmd_line_params(const int argc, const char* const * const argv) {
		exe_path = argv[0];

		if (argc > 1) {
			if (std::string(argv[1]) == "--unit-tests-only") {
				unit_tests_only = true;
			}
			else if (std::string(argv[1]) == "--help") {
				help_only = true;
			}
			else if (std::string(argv[1]) == "-h") {
				help_only = true;
			}
			else if (augs::path_type(argv[1]).extension() == ".int") {
				editor_target = argv[1];
			}
			else if (augs::path_type(argv[1]).extension() == ".lua") {
				editor_target = argv[1];
			}
		}
	}
};