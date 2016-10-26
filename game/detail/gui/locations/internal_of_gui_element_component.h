#pragma once

class internal_of_gui_element_component {
public:
	unsigned offset_of = 0;

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C, class L>
	decltype(auto) polymorphic_call(C context, L lambda) const {
		switch (offset_of) {
		case offsetof(components::gui_element, drop_item_icon):
			return lambda(drop_item_icon);
		default: 
			ensure(false);
			return lambda(drop_item_icon);
		}
	}
};