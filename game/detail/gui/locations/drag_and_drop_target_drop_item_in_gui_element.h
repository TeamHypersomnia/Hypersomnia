#pragma once

struct drag_and_drop_target_drop_item;

struct drag_and_drop_target_drop_item_in_gui_element {
public:
	typedef drag_and_drop_target_drop_item dereferenced_type;

	bool operator==(const drag_and_drop_target_drop_item_in_gui_element b) const {
		return true;
	}

	template <class C>
	bool alive(C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(C context) const {
		return &context.get_gui_element_component().drop_item_icon;
	}
};


namespace std {
	template <>
	struct hash<drag_and_drop_target_drop_item_in_gui_element> {
		size_t operator()(const drag_and_drop_target_drop_item_in_gui_element& k) const {
			return hash<size_t>()(typeid(drag_and_drop_target_drop_item_in_gui_element).hash_code());
		}
	};
}