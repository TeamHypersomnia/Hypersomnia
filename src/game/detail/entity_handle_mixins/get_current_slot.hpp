#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_current_slot() const {
	const auto& self = *static_cast<const E*>(this);

	const auto maybe_item = self.template find<components::item>();

	if (maybe_item == nullptr) {
		return self.get_cosmos()[inventory_slot_id()];
	}

	return self.get_cosmos()[maybe_item->get_current_slot()];
}

