#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_property_command.h"

#include "application/setups/editor/detail/field_address.h"
#include "application/setups/editor/property_editor/on_field_address.h"

struct entity_property_id {
	unsigned component_id = static_cast<unsigned>(-1);
	field_address field;

	template <class C, class Container, class F>
	bool access(
		C& cosm,
		const entity_type_id type_id,
		const Container& entity_ids,
		F callback
	) const {
		bool reinfer = false;

		get_by_dynamic_id(
			all_entity_types(),
			type_id,
			[&](auto e) {
				using E = decltype(e);

				get_by_dynamic_index(
					components_of<E> {},
					component_id,
					[&](const auto& c) {
						using Component = remove_cref<decltype(c)>;

						for (const auto& e : entity_ids) {
							auto specific_handle = cosm[typed_entity_id<E>(e)];

							const auto result = on_field_address(
								std::get<Component>(specific_handle.get({}).components),
								field,
								[&](auto& resolved_field) -> callback_result {
									return callback(resolved_field);
								}
							);

							if (callback_result::ABORT == result) {
								break;
							}
						}

						if (should_reinfer_after_change(c)) {
							reinfer = true;
						}
					}
				);
			}
		);

		return reinfer;
	}
};

using affected_entities_type = std::vector<entity_id_base>;

struct change_entity_property_command : change_property_command<change_entity_property_command> {
	using introspect_base = change_property_command<change_entity_property_command>;

	// GEN INTROSPECTOR struct change_entity_property_command
	entity_type_id type_id;
	affected_entities_type affected_entities;
	entity_property_id property_id;
	// END GEN INTROSPECTOR

	template <class E>
	void set_affected_entities(std::vector<typed_entity_id<E>>&& typed) {
		/* YOLO */
		reinterpret_cast<std::vector<typed_entity_id<E>>&>(affected_entities) = std::move(typed);
	}

	auto count_affected() const {
		return affected_entities.size();
	}

	template <class T, class F>
	void access_each_property(
		T in,
		F&& callback
	) const {
		auto& cosm = in.get_cosmos();

		if (property_id.access(cosm, type_id, affected_entities, continue_if_nullptr(std::forward<F>(callback)))) {
			cosmic::reinfer_all_entities(cosm);
		}
	}
};
