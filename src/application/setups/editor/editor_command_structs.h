#pragma once
#include "augs/misc/time_utils.h"

class editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}

struct editor_command_input {
	sol::state& lua;
	editor_folder& folder;
	editor_entity_selector& selector;

	void purge_selections() const;
};

struct editor_command_common {
	// GEN INTROSPECTOR struct editor_command_common
	augs::date_time timestamp;
	// END GEN INTROSPECTOR
};
