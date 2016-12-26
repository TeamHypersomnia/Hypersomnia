#pragma once
#include "augs/ensure.h"

class menu_button;

class menu_button_location {
public:
	menu_button* button = nullptr;

	const menu_button_location& get_location() const {
		return *this;
	}

	bool operator==(const menu_button_location& b) const {
		return button == b.button;
	}

	template <class C>
	bool alive(C context) const {
		return button != nullptr;
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return button;
	}
};

namespace std {
	template <>
	struct hash<menu_button_location> {
		std::size_t operator()(const menu_button_location& k) const {
			return std::hash<menu_button*>()(k.button);
		}
	};
}