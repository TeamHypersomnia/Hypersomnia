#pragma once
#include <optional>
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

struct edited_field_id {
	using index_type = unsigned;

	// GEN INTROSPECTOR struct edited_field_id
	unsigned introspective_index = static_cast<unsigned>(-1);
	std::optional<unsigned> element_index;
	// END GEN INTROSPECTOR

	bool operator==(const edited_field_id& b) const {
		return 
			introspective_index == b.introspective_index
			&& element_index == b.element_index	
		;
	}
};

