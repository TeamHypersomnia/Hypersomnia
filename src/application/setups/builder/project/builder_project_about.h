#pragma once
#include "augs/misc/constant_size_string.h"

struct builder_project_role_info {
	// GEN INTROSPECTOR struct builder_project_role_info
	augs::constant_size_string<40> role = "Role";
	augs::constant_size_string<40> person = "Person";
	// END GEN INTROSPECTOR
};

/* Less than 2 kB */

struct builder_project_about {
	// GEN INTROSPECTOR struct builder_project_about
	augs::constant_size_vector<builder_project_role_info, 10> credits;

	augs::constant_size_string<100> short_description;
	augs::constant_size_string<1000> full_description;

	augs::constant_size_string<20> game_version;
	// END GEN INTROSPECTOR
};
