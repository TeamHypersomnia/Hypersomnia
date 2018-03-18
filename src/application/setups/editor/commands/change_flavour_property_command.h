#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_flavour_id.h"
#include "application/setups/editor/editor_command_structs.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/change_property_command.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct flavour_property_id {
	unsigned invariant_id = static_cast<unsigned>(-1);
	field_address field;

	template <class C, class F>
	bool access(
		C& cosm,
		const entity_flavour_id flavour_id,
		F callback
	) const {
		bool result = false;

		cosm.change_common_significant([&](auto& common_signi) {
			common_signi.on_flavour(
				flavour_id,
				[&](auto& flavour) {
					get_by_dynamic_index(
						flavour.invariants,
						invariant_id,
						[&](auto& invariant) {
							on_field_address(
								invariant,
								field,
								[&](auto& resolved_field) {
									callback(resolved_field);
								}
							);

							if (should_reinfer_after_change(invariant)) {
								result = true;
							}
						}
					);
				}
			);

			return changer_callback_result::DONT_REFRESH;
		});

		return result;
	}
};

struct change_flavour_property_command : change_property_command<change_flavour_property_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_flavour_property_command
	// INTROSPECT BASE change_property_command<change_flavour_property_command>
	std::vector<entity_flavour_id> affected_flavours;
	flavour_property_id property_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_flavours.size();
	}

	template <class C, class F>
	void access_each_property(
		C& cosm,
		F&& callback
	) const {
		bool should_reinfer = false;

		for (const auto& a : affected_flavours) {
			if (property_id.access(cosm, a, std::forward<F>(callback))) {
				should_reinfer = true;
			}
		}

		if (should_reinfer) {
			cosm.change_common_significant([&](auto& common_signi) { return changer_callback_result::REFRESH; });
		}
	}
};
