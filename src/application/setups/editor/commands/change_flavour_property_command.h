#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_flavour_id.h"
#include "application/setups/editor/editor_command_structs.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/change_property_command.h"

struct flavour_property_id {
	entity_flavour_id flavour_id;
	unsigned invariant_id = static_cast<unsigned>(-1);
	field_address field;
};

struct change_flavour_property_command : change_property_command<change_flavour_property_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_flavour_property_command
	// INTROSPECT BASE change_property_command<change_flavour_property_command>
	flavour_property_id property_id;
	// END GEN INTROSPECTOR

	template <class C, class F>
	void access_property(
		C& cosm,
		F callback
	) {
		cosm.change_common_significant([&](auto& common_signi) {
			auto result = changer_callback_result::DONT_REFRESH;

			common_signi.on_flavour(
				property_id.flavour_id,
				[&](auto& flavour) {
					get_by_dynamic_index(
						flavour.invariants,
						property_id.invariant_id,
						[&](auto& invariant) {
							get_by_dynamic_id(
								edited_field_type_id::list_type(),
								property_id.field.type_id,
								[&](const auto& t) {
									using T = std::decay_t<decltype(t)>;

									const auto invariant_location = reinterpret_cast<std::byte*>(std::addressof(invariant));
									const auto location = reinterpret_cast<T*>(invariant_location + property_id.field.offset);

									result = callback(*location, invariant);
								}
							);
						}
					);
				}
			);

			return result;
		});
	}
};
