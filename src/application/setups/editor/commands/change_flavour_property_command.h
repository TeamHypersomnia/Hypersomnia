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

	template <class C, class Container, class F>
	bool access(
		C& cosm,
		const entity_type_id type_id,
		const Container& flavour_ids,
		F callback
	) const {
		bool result = false;

		cosm.change_common_significant([&](auto& common_signi) {
			get_by_dynamic_id(
				all_entity_types(),
				type_id,
				[&](auto e) {
					using E = decltype(e);

					get_by_dynamic_index(
						typename E::invariants {},
						invariant_id,
						[&](const auto& i) {
							using Invariant = std::decay_t<decltype(i)>;

							for (const auto& f : flavour_ids) {
								on_field_address(
									std::get<Invariant>(common_signi.template get_flavours<E>().get_flavour(f).invariants),
									field,

									[&](auto& resolved_field) {
										callback(resolved_field);
									}
								);
							}

							if (should_reinfer_after_change(i)) {
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

using affected_flavours_type = std::vector<raw_entity_flavour_id>;

struct change_flavour_property_command : change_property_command<change_flavour_property_command> {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_flavour_property_command
	// INTROSPECT BASE change_property_command<change_flavour_property_command>
	entity_type_id type_id;
	affected_flavours_type affected_flavours;
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
		if (property_id.access(cosm, type_id, affected_flavours, std::forward<F>(callback))) {
			cosm.change_common_significant([&](auto& common_signi) { return changer_callback_result::REFRESH; });
		}
	}
};
