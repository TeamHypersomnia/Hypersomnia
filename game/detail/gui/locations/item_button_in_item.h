#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/ensure.h"

class item_button_in_item {
public:
	entity_id item_id;

	const item_button_in_item& get_location() const {
		return *this;
	}

	bool operator==(const item_button_in_item& b) const {
		return item_id == b.item_id;
	}

	template <class C>
	bool alive(C context) const {
		const auto handle = context.get_step().get_cosmos()[item_id];
		return handle.alive() && context.get_gui_element_entity() == handle.get_owning_transfer_capability();
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		const auto handle = context.get_step().get_cosmos()[item_id];
		ensure(context.get_gui_element_entity() == handle.get_owning_transfer_capability());
		return &handle.get<components::item>().button;
	}
};

namespace std {
	template <>
	struct hash<item_button_in_item> {
		std::size_t operator()(const item_button_in_item& k) const {
			return std::hash<entity_id>()(k.item_id);
		}
	};
}