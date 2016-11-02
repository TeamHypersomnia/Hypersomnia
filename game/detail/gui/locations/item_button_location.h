#pragma once
#include "game/transcendental/entity_id.h"

class item_button_location {
public:
	entity_id item_id;

	const item_button_location& get_location() const {
		return *this;
	}

	bool operator==(const item_button_location& b) const {
		return item_id == b.item_id;
	}

	template <class C>
	bool alive(C context) const {
		return context.get_step().get_cosmos()[item_id].alive();
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_step().get_cosmos()[item_id].get<components::item>().button;
	}
};

namespace std {
	template <>
	struct hash<item_button_location> {
		std::size_t operator()(const item_button_location& k) const {
			return std::hash<entity_id>()(k.item_id);
		}
	};
}