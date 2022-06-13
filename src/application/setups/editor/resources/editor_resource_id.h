#pragma once
#include <compare>
#include "augs/templates/type_in_list_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/all_editor_resource_types.h"

using editor_resource_type_id = type_in_list_id<all_editor_resource_types>;

struct editor_resource_id {
	editor_resource_pool_id raw;
	editor_resource_type_id type_id;

	bool operator==(const editor_resource_id&) const = default;

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
	struct hash<editor_resource_id> {
		std::size_t operator()(const editor_resource_id v) const {
			return v.hash();
		}
	};
}
