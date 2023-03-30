#pragma once

#if 0
/*
	We determined we actually need to map to the generic editor_resource_id.
	That is because we're resolving the actual node types by just resource names.
*/

#include "augs/templates/per_type.h"

template <class T>
using make_string_to_id_map = std::unordered_map<std::string, editor_typed_resource_id<T>>;

using resource_name_to_id = per_type_container<all_editor_resource_types, make_string_to_id_map>;
#else
using resource_name_to_id = std::unordered_map<std::string, editor_resource_id>;
#endif

auto editor_official_resource_map::create_name_to_id_map() const {
	resource_name_to_id name_to_id;

	auto map_names = [&](const auto&, auto& m) {
		for (const auto& entry : m) {
			const auto name = to_lowercase(augs::enum_to_string(entry.first));
			name_to_id.try_emplace(name, entry.second.operator editor_resource_id());
		}
	};

	augs::introspect(map_names, *this);

	return name_to_id;
}
