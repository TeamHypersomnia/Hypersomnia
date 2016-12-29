#pragma once
class app_ui_root;

class app_ui_root_in_context {
public:
	typedef app_ui_root dereferenced_type;

	bool operator==(app_ui_root_in_context b) const {
		return true;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_root_of_app_ui();
	}
};

namespace std {
	template <>
	struct hash<app_ui_root_in_context> {
		size_t operator()(const app_ui_root_in_context& k) const {
			return hash<size_t>()(typeid(app_ui_root_in_context).hash_code());
		}
	};
}