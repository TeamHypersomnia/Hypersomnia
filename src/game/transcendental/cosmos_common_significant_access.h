#pragma once

struct flavour_property_id;

struct create_flavour_command;
struct delete_flavour_command;

class cosmos_common_significant_access {
	friend flavour_property_id;

	/* Some classes for editor must be privileged */
	friend create_flavour_command;
	friend delete_flavour_command;

	cosmos_common_significant_access() {}
};
