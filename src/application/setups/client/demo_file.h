#pragma once
#include <vector>
#include "hypersomnia_version.h"

struct demo_file_meta {
	// GEN INTROSPECTOR struct demo_file_meta
	hypersomnia_version version;
	std::string server_address;
	// END GEN INTROSPECTOR
};

struct demo_step;

struct demo_file {
	demo_file_meta meta;
};
