#pragma once
#include <cstddef>
#include "game/cosmos/entity_id.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "augs/templates/hash_fwd.h"

struct item_button;

class item_button_in_item {
public:
	using dereferenced_type = item_button;

	entity_id item_id;

	const item_button_in_item& get_location() const {
		return *this;
	}

	bool operator==(const item_button_in_item& b) const {
		return item_id == b.item_id;
	}

	template <class C>
	bool alive(const C context) const {
		const auto handle = context.get_cosmos()[item_id];
		return handle.alive() && context.get_subject_entity() == handle.get_owning_transfer_capability();
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_game_gui_system().get_item_button(item_id);
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