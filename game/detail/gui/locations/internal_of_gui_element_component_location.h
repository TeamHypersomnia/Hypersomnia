#pragma once

class internal_of_gui_element_component_location {
public:
	components::gui_element::internal_element element = components::gui_element::internal_element::INVALID;

	bool operator==(internal_of_gui_element_component_location b) const {
		return element == b.element;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C, class L>
	decltype(auto) get_object_at_location_and_call(C context, L generic_call) const {
		using namespace components;

		auto& elem = context.get_gui_element_component();
		
		switch (element) {
			case gui_element::internal_element::DROP_ITEM_ICON:
			return generic_call(elem.drop_item_icon);
		default: 
			ensure(false);
			return generic_call(elem.drop_item_icon);
		}
	}
};

namespace std {
	template <>
	struct hash<internal_of_gui_element_component_location> {
		std::size_t operator()(const internal_of_gui_element_component_location& k) const {
			return std::hash<decltype(k.element)>()(k.element);
		}
	};
}