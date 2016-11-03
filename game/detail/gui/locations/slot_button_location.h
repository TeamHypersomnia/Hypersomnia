#pragma once
#include "game/detail/inventory_slot_id.h"

class slot_button_location {
public:
	inventory_slot_id slot_id;

	bool operator==(const slot_button_location& b) const {
		return slot_id == b.slot_id;
	}

	template <class C>
	bool alive(C context) const {
		const auto handle = context.get_step().get_cosmos()[slot_id];
		return handle.alive() && context.get_gui_element_entity() == handle.get_container().get_owning_transfer_capability();
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_step().get_cosmos()[slot_id]->button;
	}
};

namespace std {
	template <>
	struct hash<slot_button_location> {
		std::size_t operator()(const slot_button_location& k) const {
			return std::hash<inventory_slot_id>()(k.slot_id);
		}
	};
}