#pragma once
#include <cstddef>
#include <vector>

using edited_mode_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string
	>
>;

struct mode_field_address {
	// GEN INTROSPECTOR struct mode_field_address
	unsigned offset = static_cast<unsigned>(-1);
	unsigned element_index = static_cast<unsigned>(-1);
	edited_mode_field_type_id type_id;
	// END GEN INTROSPECTOR

	bool operator==(const mode_field_address& b) const {
		return offset == b.offset && element_index && b.element_index && type_id == b.type_id;
	}

	bool operator!=(const mode_field_address& b) const {
		return !operator==(b);
	}
};
namespace mode_messages {
	struct change_var {
		mode_field_address addr;
		std::vector<std::byte> value;
	};
};
