#pragma once
#include "augs/ensure.h"
#include "application/menu_button_type.h"

class option_button;

class option_button_in_menu {
public:
	using dereferenced_type = option_button;
	menu_button_type type = menu_button_type::COUNT;

	const option_button_in_menu& get_location() const {
		return *this;
	}

	bool operator==(const option_button_in_menu& b) const {
		return type == b.type;
	}

	template <class C>
	bool alive(const C context) const {
		return type != menu_button_type::COUNT;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root_of_app_ui().menu_buttons.at(static_cast<size_t>(type));
	}
};

namespace std {
	template <>
	struct hash<option_button_in_menu> {
		std::size_t operator()(const option_button_in_menu& k) const {
			return std::hash<menu_button_type>()(k.type);
		}
	};
}