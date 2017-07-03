#pragma once
class menu_ui_root;

class menu_ui_root_in_context {
public:
	using dereferenced_type = menu_ui_root;

	bool operator==(menu_ui_root_in_context b) const {
		return true;
	}

	template <class C>
	bool alive(const C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root_of_menu_ui();
	}
};

namespace std {
	template <>
	struct hash<menu_ui_root_in_context> {
		size_t operator()(const menu_ui_root_in_context& k) const {
			return hash<size_t>()(typeid(menu_ui_root_in_context).hash_code());
		}
	};
}