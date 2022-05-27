#pragma once

struct builder_project_role_info {
	// GEN INTROSPECTOR struct builder_project_role_info
	std::string role = "Role";
	std::string person = "Person";
	// END GEN INTROSPECTOR
};

struct builder_project_about {
	// GEN INTROSPECTOR struct builder_project_about
	std::vector<builder_project_role_info> credits;

	std::string short_description;
	std::string full_description;
	// END GEN INTROSPECTOR
};
