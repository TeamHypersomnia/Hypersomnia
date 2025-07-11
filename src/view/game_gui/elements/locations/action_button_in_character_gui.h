#pragma once
#include <cstddef>

class action_button;

struct action_button_in_character_gui {
public:
	using dereferenced_type = action_button;

	int index = -1;

	bool operator==(const action_button_in_character_gui b) const {
		return index == b.index;
	}

	template <class C>
	bool alive(const C context) const {
		return index >= 0 && index < static_cast<int>(context.get_character_gui().action_buttons.size());
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().action_buttons[index];
	}
};


namespace std {
	template <>
	struct hash<action_button_in_character_gui> {
		size_t operator()(const action_button_in_character_gui& k) const {
			return std::hash<int>()(k.index);
		}
	};
}