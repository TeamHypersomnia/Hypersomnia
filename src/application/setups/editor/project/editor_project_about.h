#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/misc/constant_size_vector.h"

struct editor_project_role_info {
	// GEN INTROSPECTOR struct editor_project_role_info
	augs::constant_size_string<40> role = "Role";
	augs::constant_size_string<40> person = "Person";
	// END GEN INTROSPECTOR
};

/* Less than 2 kB */

struct editor_project_about {
	// GEN INTROSPECTOR struct editor_project_about
	augs::constant_size_vector<editor_project_role_info, 10> credits;

	augs::constant_size_string<40> full_name;
	augs::constant_size_string<200> short_description;
	augs::constant_size_string<1000> full_description;
	// END GEN INTROSPECTOR
};
