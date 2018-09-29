#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_owning_transfer_capability() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	if (self.dead()) {
		return cosm[entity_id()];
	}

	if (self.template has<components::item_slot_transfers>()) {
		return self;
	}

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosm[item->get_current_slot()]) {
			return slot.get_container().get_owning_transfer_capability();
		}
	}

	return cosm[entity_id()];
}

