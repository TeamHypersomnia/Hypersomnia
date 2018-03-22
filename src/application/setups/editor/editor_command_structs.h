#pragma once
#include <optional>
#include "augs/misc/time_utils.h"

class editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}
struct editor_all_entities_gui;

struct editor_command_input {
	sol::state& lua;
	editor_folder& folder;
	editor_entity_selector& selector;

	editor_all_entities_gui& all_entities_gui;

	void purge_selections() const;
	void interrupt_tweakers() const;
};

struct editor_command_common {
	// GEN INTROSPECTOR struct editor_command_common
	std::time_t timestamp = {};
	// END GEN INTROSPECTOR

	void reset_timestamp() {
		timestamp = augs::date_time();
	}
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

