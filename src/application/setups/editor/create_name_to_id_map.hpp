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

template <class S>
auto editor_official_resource_map::create_id_to_name_map(S& taken_names) const {
	std::unordered_map<editor_resource_id, std::string> id_to_name;

	auto map_ids = [&](const auto&, auto& m) {
		for (const auto& entry : m) {
			const auto name = to_lowercase(augs::enum_to_string(entry.first));
			id_to_name.try_emplace(entry.second.operator editor_resource_id(), name);

			taken_names.emplace(name);
		}
	};

	augs::introspect(map_ids, *this);

	return id_to_name;
}

inline auto editor_official_resource_map::create_name_to_id_map() const {
	resource_name_to_id name_to_id;

	auto map_names = [&](const auto&, auto& m) {
		for (const auto& entry : m) {
			const auto name = to_lowercase(augs::enum_to_string(entry.first));
			LOG("Registering %x", name);
			const auto it = name_to_id.try_emplace(name, entry.second.operator editor_resource_id());

			(void)it;
			/* Materials have very generic names and they won't be used in ambiguous contexts */
			if constexpr(!std::is_same_v<remove_cref<decltype(entry.first)>, test_scene_physical_material_id>) {
				if (!it.second) {
					LOG("DUPLICATE OFFICIAL NAMES! %x", name);

					ensure(false && "DUPLICATE OFFICIAL NAMES");
				}
			}
		}
	};

	augs::introspect(map_names, *this);

	constexpr std::pair<const char*, const char*> redirects[4] = {
		{ "hpsr_magazine", "awka_magazine" },

		{ "portal_enter", "portal_enter_particles" },
		{ "portal_exit", "portal_exit_particles" },
		{ "portal_begin_entering", "portal_begin_entering_particles" },
	};

	for (const auto& r : redirects) {
		if (const auto redirect = mapped_or_nullptr(name_to_id, r.second)) {
			name_to_id[r.first] = *redirect;
		}
	}

	return name_to_id;
}
