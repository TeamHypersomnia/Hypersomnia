#pragma once

class root_of_inventory_gui;

class root_of_inventory_gui_in_context {
public:
	using dereferenced_type = root_of_inventory_gui;

	bool operator==(root_of_inventory_gui_in_context b) const {
		return true;
	}

	template <class C>
	bool alive(const C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root_of_inventory_gui();
	}
};

namespace std {
	template <>
	struct hash<root_of_inventory_gui_in_context> {
		size_t operator()(const root_of_inventory_gui_in_context& k) const {
			return hash<size_t>()(typeid(root_of_inventory_gui_in_context).hash_code());
		}
	};
}