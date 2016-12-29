#pragma once
#include "augs/ensure.h"
#include "application/menu_button_type.h"

class dx_button_in_menu {
public:
	menu_button_type type;

	const dx_button_in_menu& get_location() const {
		return *this;
	}

	bool operator==(const dx_button_in_menu& b) const {
		return type == b.type;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_root_of_app_ui().menu_buttons[type];
	}
};

namespace std {
	template <>
	struct hash<dx_button_in_menu> {
		std::size_t operator()(const dx_button_in_menu& k) const {
			return std::hash<menu_button_type>()(k.type);
		}
	};
}