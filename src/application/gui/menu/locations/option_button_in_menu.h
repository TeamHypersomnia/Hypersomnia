#pragma once
#include "augs/ensure.h"

template <class Enum, class Button>
class option_button_in_menu {
public:
	using dereferenced_type = Button;
	Enum type = Enum::COUNT;

	const auto& get_location() const {
		return *this;
	}

	bool operator==(const option_button_in_menu& b) const {
		return type == b.type;
	}

	template <class C>
	bool alive(const C) const {
		return type != Enum::COUNT;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root().buttons.at(static_cast<std::size_t>(type));
	}
};

namespace std {
	template <class Enum, class Button>
	struct hash<option_button_in_menu<Enum, Button>> {
		std::size_t operator()(const option_button_in_menu<Enum, Button>& k) const {
			return std::hash<Enum>()(k.type);
		}
	};
}

