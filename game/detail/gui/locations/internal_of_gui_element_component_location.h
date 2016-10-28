#pragma once

class internal_of_gui_element_component_location {
public:
	unsigned offset_of = 0;

	bool operator==(internal_of_gui_element_component_location b) const {
		return offset_of == b.offset_of;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C, class L>
	decltype(auto) get_object_at_location_and_call(C context, L polymorphic_call) const {
		auto& elem = context.get_gui_element_component();
		
		switch (offset_of) {
		case offsetof(components::gui_element, drop_item_icon):
			return polymorphic_call(elem.drop_item_icon);
		default: 
			ensure(false);
			return polymorphic_call(elem.drop_item_icon);
		}
	}
};

namespace std {
	template <>
	struct hash<internal_of_gui_element_component_location> {
		std::size_t operator()(const internal_of_gui_element_component_location& k) const {
			return std::hash<unsigned>()(k.offset_of);
		}
	};
}