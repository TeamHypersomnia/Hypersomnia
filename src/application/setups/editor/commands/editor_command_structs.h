#pragma once
#include <optional>
#include "augs/misc/time_utils.h"

struct editor_command_common {
	// GEN INTROSPECTOR struct editor_command_common
	std::time_t timestamp = {};
	bool has_parent = false;
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

