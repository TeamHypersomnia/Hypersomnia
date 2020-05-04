#pragma once

struct builder_project_role_info {
	// GEN INTROSPECTOR struct builder_project_role_info
	std::string person = "Person";
	std::string role = "Role";
	// END GEN INTROSPECTOR
};

struct builder_project_meta {
	// GEN INTROSPECTOR struct builder_project_meta
	std::vector<builder_project_role_info> roles;
	// END GEN INTROSPECTOR
};
