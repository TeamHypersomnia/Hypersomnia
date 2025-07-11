#pragma once
#include <cstddef>

class game_gui_root;

class game_gui_root_in_context {
public:
	using dereferenced_type = game_gui_root;

	bool operator==(game_gui_root_in_context b) const {
		return true;
	}

	template <class C>
	bool alive(const C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_root();
	}
};

namespace std {
	template <>
	struct hash<game_gui_root_in_context> {
		size_t operator()(const game_gui_root_in_context& k) const {
			return hash<size_t>()(typeid(game_gui_root_in_context).hash_code());
		}
	};
}