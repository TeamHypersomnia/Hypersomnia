#pragma once
#include <compare>
#include "augs/templates/type_in_list_id.h"
#include "augs/misc/pool/pooled_object_id.h"
#include "application/setups/editor/nodes/all_editor_node_types_declaration.h"

using editor_node_pool_size_type = unsigned short;
using editor_node_type_id = type_in_list_id<all_editor_node_types>;
using editor_node_pool_id = augs::pooled_object_id<editor_node_pool_size_type>;

struct editor_node_id {
	editor_node_pool_id raw;
	editor_node_type_id type_id;

	bool operator==(const editor_node_id&) const = default;

	void unset() {
		raw.unset();
		type_id.unset();
	}

	bool is_set() const {
		return raw.is_set() && type_id.is_set();
	}

	auto hash() const {
		return augs::hash_multiple(raw, type_id);
	}
};

namespace std {
	template <>
	struct hash<editor_node_id> {
		std::size_t operator()(const editor_node_id v) const {
			return v.hash();
		}
	};
}
