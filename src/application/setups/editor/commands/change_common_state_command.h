#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "application/setups/editor/editor_command_structs.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/change_property_command.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct change_common_state_command : change_property_command<change_common_state_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_common_state_command
	// INTROSPECT BASE change_property_command<change_common_state_command>
	field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}

	template <class C, class F>
	void access_each_property(
		C& cosm,
		F callback
	) const {
		cosm.change_common_significant([&](auto& common_signi) {
			on_field_address(
				common_signi,
				field,
				[&](auto& resolved_field) {
					callback(resolved_field);
				}
			);

			return changer_callback_result::DONT_REFRESH;
		});
	}
};
