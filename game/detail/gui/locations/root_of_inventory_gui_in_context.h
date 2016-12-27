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

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_root_of_inventory_gui();
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