#pragma once

class root_of_inventory_gui_location {
public:
	bool operator==(root_of_inventory_gui_location b) const {
		return true;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C, class L>
	decltype(auto) get_object_at_location_and_call(C context, L generic_call) const {
		root_of_inventory_gui node;
		return generic_call(node);
	}
};

namespace std {
	template <>
	struct hash<root_of_inventory_gui_location> {
		size_t operator()(const root_of_inventory_gui_location& k) const {
			return hash<size_t>()(typeid(root_of_inventory_gui_location).hash_code());
		}
	};
}