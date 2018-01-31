#pragma once
#include <vector>

#include "augs/callback_result.h"
#include "augs/build_settings/platform_defines.h"

#include "augs/templates/introspect_declaration.h"
#include "augs/templates/maybe_const.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/child_entity_name.h"

struct entity_relations;

template <class entity_handle_type>
class relations_mixin {
protected:
	static constexpr bool is_const = is_class_const_v<entity_handle_type>;

	using inventory_slot_handle_type = basic_inventory_slot_handle<entity_handle_type>;

	auto* get_id_ptr(const child_entity_name n) const {
		const auto& self = *static_cast<const entity_handle_type*>(this);

		maybe_const_ptr_t<is_const, child_entity_id> result = nullptr;

		if (self.alive()) {
			switch (n) {
				case child_entity_name::CROSSHAIR_RECOIL_BODY:
				if (const auto crosshair = self.template find<components::crosshair>()) {
					result = std::addressof(crosshair->recoil_entity);
				}
				break;

				case child_entity_name::CHARACTER_CROSSHAIR:
				if (const auto sentience = self.template find<components::sentience>()) {
					result = std::addressof(sentience->character_crosshair);
				}
				break;

				default:
				LOG("Random access abstraction for this child_entity_name is not implemented!");
				ensure(false);
				break;
			}
		}

		return result;
	}

public:
	entity_handle_type get_parent() const;
	
	inventory_slot_handle_type operator[](const slot_function) const;
	entity_handle_type operator[](const child_entity_name) const;
	
	template <class F>
	void for_each_child_entity_recursive(F&& callback) const {
		const auto self = *static_cast<const entity_handle_type*>(this);
		auto& cosmos = self.get_cosmos();

		self.for_each_component(
			[&cosmos, &callback](auto& subject_component) {
				augs::introspect(
					[&](auto, auto& member) {
						if constexpr(std::is_same_v<std::decay_t<decltype(member)>, child_entity_id>) {
							const auto child_handle = cosmos[member];

							if (child_handle.alive() && callback(child_handle) == callback_result::CONTINUE) {
								child_handle.for_each_child_entity_recursive(std::forward<F>(callback));
							}
						}
					},
					subject_component
				);
			}
		);
	}

	void make_as_child_of(const entity_id) const;
	void map_child_entity(const child_entity_name n, const entity_id p) const;
};

template <class E>
void relations_mixin<E>::make_as_child_of(const entity_id parent_id) const {
	auto& self = *static_cast<const E*>(this);

	auto& ch = self += components::existential_child();
	ch.parent = parent_id;
}

template <class E>
void relations_mixin<E>::map_child_entity(
	const child_entity_name n, 
	const entity_id p
) const {
	if (const auto maybe_id = get_id_ptr(n)) {
		*maybe_id = p;
	}
}

template <class E>
typename relations_mixin<E>::inventory_slot_handle_type relations_mixin<E>::operator[](const slot_function func) const {
	auto& self = *static_cast<const E*>(this);
	return inventory_slot_handle_type(self.get_cosmos(), inventory_slot_id(func, self.get_id()));
}

template <class E>
E relations_mixin<E>::operator[](const child_entity_name child) const {
	auto& self = *static_cast<const E*>(this);

	entity_id id;

	if (const auto maybe_id = get_id_ptr(child)) {
		id = *maybe_id;
	}

	return self.get_cosmos()[id];
}

template <class E>
E relations_mixin<E>::get_parent() const {
	auto& self = *static_cast<const E*>(this);

	return self.get_cosmos()[self.template get<components::existential_child>().parent];
}
