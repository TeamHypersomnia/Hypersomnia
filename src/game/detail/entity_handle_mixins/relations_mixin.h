#pragma once
#include <vector>

#include "augs/enums/callback_result.h"
#include "augs/build_settings/platform_defines.h"

#include "augs/templates/introspect_declaration.h"
#include "augs/templates/maybe_const.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/child_entity_name.h"

struct entity_relations;

template <class entity_handle_type>
class relations_mixin {
protected:
	static constexpr bool is_const = is_handle_const_v<entity_handle_type>;
	using generic_handle_type = basic_entity_handle<is_const>;
	using inventory_slot_handle_type = basic_inventory_slot_handle<generic_handle_type>;

	template <class F>
	void on_id(const child_entity_name n, F) const {
		const auto& self = *static_cast<const entity_handle_type*>(this);

		if (self.alive()) {
			switch (n) {
				default:
				LOG("Random access abstraction for this child_entity_name is not implemented!");
				ensure(false);
				break;
			}
		}
	}

public:
	inventory_slot_handle_type operator[](const slot_function) const;
	generic_handle_type operator[](const child_entity_name) const;
	
	template <class F>
	void for_each_child_entity_recursive(F&& callback) const {
		const auto self = *static_cast<const entity_handle_type*>(this);
		auto& cosmos = self.get_cosmos();

		self.for_each_component(
			[&cosmos, &callback](const auto& subject_component) {
				augs::introspect(
					[&](auto, const auto& member) {
						if constexpr(std::is_same_v<remove_cref<decltype(member)>, child_entity_id>) {
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

	void map_child_entity(const child_entity_name n, const entity_id p) const;
};

template <class E>
void relations_mixin<E>::map_child_entity(
	const child_entity_name n, 
	const entity_id p
) const {
	on_id(n, [p](auto& id){ id = p; });
}

template <class E>
typename relations_mixin<E>::inventory_slot_handle_type relations_mixin<E>::operator[](const slot_function func) const {
	auto& self = *static_cast<const E*>(this);
	return inventory_slot_handle_type(self.get_cosmos(), inventory_slot_id(func, self.get_id()));
}

template <class E>
typename relations_mixin<E>::generic_handle_type relations_mixin<E>::operator[](const child_entity_name child) const {
	auto& self = *static_cast<const E*>(this);

	entity_id result;

	on_id(child, [&result](const auto& id){ result = id; });

	return self.get_cosmos()[result];
}
