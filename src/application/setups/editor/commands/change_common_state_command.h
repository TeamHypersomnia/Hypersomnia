#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/cosmic_functions.h"

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_property_command.h"

#include "application/setups/editor/detail/field_address.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct change_common_state_command : change_property_command<change_common_state_command> {
	friend augs::introspection_access;

	using introspect_base = change_property_command<change_common_state_command>;

	// GEN INTROSPECTOR struct change_common_state_command
	field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}

	template <class T, class F>
	void access_each_property(
		T in,
		F callback
	) const {
		auto& cosm = in.get_cosmos();

		cosm.change_common_significant([&](auto& common_signi) {
			on_field_address(
				common_signi,
				field,
				continue_if_nullptr([&](auto& resolved_field) {
					return callback(resolved_field);
				})
			);

			return changer_callback_result::DONT_REFRESH;
		});
	}
};
