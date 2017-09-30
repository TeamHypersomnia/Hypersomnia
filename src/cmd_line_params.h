#pragma once
#include "augs/misc/typesafe_sscanf.h"
#include "augs/filesystem/path.h"

struct cmd_line_params {
	augs::path_type editor_target;

	cmd_line_params(const int argc, const char* const * const argv) {
		for (int i = 0; i < argc; ++i) {
			const auto argv_i = std::string(argv[i]);

			if (const auto it = argv_i.find("--edit=");
				it != std::string::npos
			) {
				typesafe_sscanf(argv_i, "--edit=%x", editor_target);
			}
		}
	}
};