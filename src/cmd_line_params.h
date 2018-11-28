#pragma once
#include "augs/string/typesafe_sscanf.h"
#include "augs/filesystem/path.h"

struct cmd_line_params {
	augs::path_type exe_path;
	augs::path_type editor_target;
	bool unit_tests_only = false;
	bool help_only = false;
	bool start_server = false;
	bool should_connect = false;
	std::string connect_to_address;

	cmd_line_params(const int argc, const char* const * const argv) {
		exe_path = argv[0];

		if (argc > 1) {
			const auto a = std::string(argv[1]);

			if (a == "--unit-tests-only") {
				unit_tests_only = true;
			}
			else if (a == "--help" || a == "-h") {
				help_only = true;
			}
			else if (a == "--start-server") {
				start_server = true;
			}
			else if (a == "--connect") {
				should_connect = true;
				
				if (argc > 2) {
					connect_to_address = argv[2];
				}
			}
			else {
				editor_target = a;
			}
		}
	}
};