#pragma once

class hotbar_button;

struct hotbar_button_in_gui_element {
public:
	typedef hotbar_button dereferenced_type;

	int index = -1;

	bool operator==(const hotbar_button_in_gui_element b) const {
		return index == b.index;
	}

	template <class C>
	bool alive(C context) const {
		return index >= 0 && index < static_cast<int>(context.get_gui_element_component().hotbar_buttons.size());
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_gui_element_component().hotbar_buttons.at(index);
	}
};


namespace std {
	template <>
	struct hash<hotbar_button_in_gui_element> {
		size_t operator()(const hotbar_button_in_gui_element& k) const {
			return std::hash<int>()(k.index);
		}
	};
}