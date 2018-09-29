#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"
#include "game/detail/inventory/direct_attachment_offset.h"

template <class A, class Csm, class E, class C>
void detail_save_and_forward(
	colliders_connection& result, 
	const A& slot,
	const Csm& cosm, 
	E& current_attachment,
	C& offsets,
	inventory_slot_id& it
) {
	const auto item_entity = cosm[current_attachment];
	const auto container_entity = slot.get_container();

	offsets.push_back(
		direct_attachment_offset(
			container_entity, 
			item_entity, 
			attachment_offset_settings::for_logic(), 
			it.type
		)
	);

	result.owner = container_entity;

	current_attachment = container_entity.get_id();
	it = container_entity.get_current_slot();
}

template <class E>
std::optional<colliders_connection> inventory_mixin<E>::calc_connection_to_topmost_container() const {
	thread_local offset_vector offsets;
	offsets.clear();

	const auto& self = *static_cast<const E*>(this);
	ensure(self);

	const auto& cosm = self.get_cosmos();

	colliders_connection result;

	auto it = self.get_current_slot().get_id();
	entity_id current_attachment = self.get_id();

	while (const auto slot = cosm[it]) {
		if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
			/* 
				Failed: "until" not found before meeting an item deposit.
				This nullopt will be used to determine
				that the fixtures for this item should be deactivated now.
			*/

			return std::nullopt;
		}

		/* 
			At this point, 
			behaviour must be slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY.
		*/

		detail_save_and_forward(result, slot, cosm, current_attachment, offsets, it);
	}

	for (const auto& o : reverse(offsets)) {
		result.shape_offset = result.shape_offset * o;
	}
	
	return result;
}

template <class E>
std::optional<colliders_connection> inventory_mixin<E>::calc_connection_until_container(const entity_id until) const {
	thread_local offset_vector offsets;
	offsets.clear();

	const auto& self = *static_cast<const E*>(this);

	ensure(self);
	ensure(until.is_set());

	const auto& cosm = self.get_cosmos();

	if (until == self) {
		return colliders_connection { self, {} };
	}

	colliders_connection result;

	auto it = self.get_current_slot().get_id();
	entity_id current_attachment = self.get_id();

	do {
		const auto slot = cosm[it];

		if (slot.dead()) {
			/* Failed: found a dead slot before could reach "until" */
			return std::nullopt;
		}

		if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
			/* Failed: "until" not found before meeting an item deposit. */
			return std::nullopt;
		}

		/* 
			At this point, 
			behaviour must be slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY.
		*/

		detail_save_and_forward(result, slot, cosm, current_attachment, offsets, it);
	} while(it.container_entity != until);

	for (const auto& o : reverse(offsets)) {
		result.shape_offset = result.shape_offset * o;
	}
	
	return result;
}

