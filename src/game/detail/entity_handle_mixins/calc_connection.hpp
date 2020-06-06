#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"
#include "game/detail/inventory/direct_attachment_offset.h"

template <class A, class Csm, class E, class C>
void detail_save_and_forward(
	entity_id& topmost_owner, 
	const A& slot,
	const Csm& cosm, 
	E& current_attachment,
	C& offsets,
	inventory_slot_id& it
) {
	const auto item_entity = cosm[current_attachment];
	const auto container_entity = slot.get_container();

	{
		const auto next_offset = direct_attachment_offset(
			container_entity, 
			item_entity, 
			attachment_offset_settings::for_logic(), 
			it.type
		);

		offsets.push_back(next_offset);
	}

	topmost_owner = container_entity;
	current_attachment = container_entity.get_id();
	it = container_entity.get_current_slot();
}

template <class E>
colliders_connection inventory_mixin<E>::calc_connection_to_topmost_container() const {
	thread_local offset_vector offsets;
	offsets.clear();

	const auto& self = *static_cast<const E*>(this);
	ensure(self);

	const auto& cosm = self.get_cosmos();

	entity_id topmost_owner;

	auto it = self.get_current_slot().get_id();
	entity_id current_attachment = self.get_id();

	while (const auto slot = cosm[it]) {
		if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
			/* 
				Failed: "until" not found before meeting an item deposit.
				This nullopt will be used to determine
				that the fixtures for this item should be deactivated now.
			*/

			return colliders_connection::none();
		}

		/* 
			At this point, 
			behaviour must be slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY.
		*/

		detail_save_and_forward(topmost_owner, slot, cosm, current_attachment, offsets, it);
	}

	auto result_offset = attachment_offset();

	for (auto o : reverse(offsets)) {
		result_offset = result_offset * o;
	}
	
	auto result = colliders_connection::none();
	result.owner = topmost_owner;
	result.shape_offset = result_offset.offset;
	result.flip_geometry = result_offset.flip_geometry;

	return result;
}

template <class E>
colliders_connection physics_mixin<E>::calc_colliders_connection() const {
	const auto self = *static_cast<const E*>(this);
	const auto& cosm = self.get_cosmos();
	
	auto result = colliders_connection::none();

	self.template dispatch_on_having_all<invariants::fixtures>([&](const auto typed_self) {
		if (const auto overridden = typed_self.template find<components::specific_colliders_connection>()) {
			result = overridden->connection;
			return;
		}

		if (const auto item = typed_self.template find<components::item>()) {
			if (const auto slot = cosm[item->get_current_slot()]) {
				result = typed_self.calc_connection_to_topmost_container();
				return;
			}
		}

		if (typed_self.template find<components::rigid_body>()) {
	#if MORE_LOGS
			LOG("%x (body) owned by itself", typed_self);
	#endif
			result.owner = typed_self;
			return;
		}
	});

	return result;
}

