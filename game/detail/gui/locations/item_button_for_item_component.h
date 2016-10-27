#pragma once
#include "game/transcendental/entity_id.h"

class item_button_for_item_component {
public:
	entity_id item_id;

	template <class C>
	bool alive(C context) const {
		return context.get_step().get_cosmos()[item_id].alive();
	}

	template <class C, class L>
	decltype(auto) get_object_at_location_and_call(C context, L polymorphic_call) const {
		return polymorphic_call(context.get_step().get_cosmos()[item_id].get<components::item>().button);
	}
};