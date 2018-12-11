#pragma once
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "augs/ensure.h"

struct slot_button;

class slot_button_in_container {
public:
	using dereferenced_type = slot_button;

	inventory_slot_id slot_id;

	bool operator==(const slot_button_in_container& b) const {
		return slot_id == b.slot_id;
	}

	template <class C>
	bool alive(const C context) const {
		const auto handle = context.get_cosmos()[slot_id];
		return handle.alive() && context.get_subject_entity() == handle.get_container().get_owning_transfer_capability();
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		const auto handle = context.get_cosmos()[slot_id];
		(void)handle;
		ensure_eq(context.get_subject_entity(), handle.get_container().get_owning_transfer_capability());
		return &context.get_game_gui_system().get_slot_button(slot_id);
	}
};

namespace std {
	template <>
	struct hash<slot_button_in_container> {
		std::size_t operator()(const slot_button_in_container& k) const {
			return std::hash<inventory_slot_id>()(k.slot_id);
		}
	};
}