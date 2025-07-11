#pragma once
#include <cstddef>

template <class Enum>
class menu_root;

template <class E>
class menu_root_in_context {
public:
	using dereferenced_type = menu_root<E>;

	bool operator==(menu_root_in_context) const {
		return true;
	}

	template <class C>
	bool alive(const C) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root();
	}
};

namespace std {
	template <class E>
	struct hash<menu_root_in_context<E>> {
		size_t operator()(const menu_root_in_context<E>&) const {
			return hash<size_t>()(typeid(menu_root_in_context<E>).hash_code());
		}
	};
}