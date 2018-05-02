#pragma once

struct drag_and_drop_target_drop_item;

struct drag_and_drop_target_drop_item_in_character_gui {
public:
	using dereferenced_type = drag_and_drop_target_drop_item;

	bool operator==(const drag_and_drop_target_drop_item_in_character_gui) const {
		return true;
	}

	template <class C>
	bool alive(const C&) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().drop_item_icon;
	}
};


namespace std {
	template <>
	struct hash<drag_and_drop_target_drop_item_in_character_gui> {
		size_t operator()(const drag_and_drop_target_drop_item_in_character_gui&) const {
			return hash<size_t>()(typeid(drag_and_drop_target_drop_item_in_character_gui).hash_code());
		}
	};
}