#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "application/setups/editor/editor_command_structs.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/change_property_command.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct entity_property_id {
	entity_id subject_id;
	unsigned component_id = static_cast<unsigned>(-1);
	field_address field;
};

struct change_entity_property_command : change_property_command<change_entity_property_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_entity_property_command
	// INTROSPECT BASE change_property_command<change_entity_property_command>
	entity_property_id property_id;
	// END GEN INTROSPECTOR

	template <class C, class F>
	void access_property(
		C& cosm,
		F callback
	) {
		auto result = changer_callback_result::DONT_REFRESH;

		const auto handle = cosm[property_id.subject_id];

		handle.dispatch([&](const auto typed_handle) {
			get_by_dynamic_index(
				typed_handle.get({}).components,
				property_id.component_id,
				[&](auto& component) {
					result = on_field_address(
						component,
						property_id.field,
						[&](auto& field) {
							return callback(field, component);
						}
					);
				}
			);
		});

		if (result != changer_callback_result::DONT_REFRESH) {
			cosmic::reinfer_all_entities(cosm);
		}
	}
};
