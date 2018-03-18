#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "application/setups/editor/editor_command_structs.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/change_property_command.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct entity_property_id {
	unsigned component_id = static_cast<unsigned>(-1);
	field_address field;

	template <class C, class F>
	bool access(
		C& cosm,
		const entity_id subject_id,
		F callback
	) const {
		const auto handle = cosm[subject_id];

		return handle.dispatch([&](const auto typed_handle) {
			return get_by_dynamic_index(
				typed_handle.get({}).components,
				component_id,
				[&](auto& component) {
					on_field_address(
						component,
						field,
						[&](auto& resolved_field) {
							callback(resolved_field);
						}
					);

					return should_reinfer_after_change(component);
				}
			);
		});
	}
};

struct change_entity_property_command : change_property_command<change_entity_property_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_entity_property_command
	// INTROSPECT BASE change_property_command<change_entity_property_command>
	std::vector<entity_id> affected_entities;
	entity_property_id property_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_entities.size();
	}

	template <class C, class F>
	void access_each_property(
		C& cosm,
		F&& callback
	) const {
		bool should_reinfer = false;

		for (const auto& a : affected_entities) {
			if (property_id.access(cosm, a, std::forward<F>(callback))) {
				should_reinfer = true;
			}
		}

		if (should_reinfer) {
			cosmic::reinfer_all_entities(cosm);
		}
	}
};
