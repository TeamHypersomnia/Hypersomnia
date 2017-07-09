#pragma once
#include "application/menu_ui/menu_ui_element_location.h"
#include "application/menu_ui/option_button.h"
#include "application/menu_button_type.h"
#include "augs/gui/dereferenced_location.h"
#include "augs/misc/enum_array.h"

class menu_ui_root : public menu_ui_rect_node {
public:
	menu_ui_root();

	augs::enum_array<option_button, menu_button_type> menu_buttons;

	void set_menu_buttons_positions(const vec2i screen_size);
	void set_menu_buttons_sizes(const vec2i size);
	void set_menu_buttons_colors(const rgba col);

	vec2i get_max_menu_button_size() const;

	template <class C, class D, class L>
	static void for_each_child(const C context, const D this_id, L generic_call) {
		option_button_in_menu loc;

		for (size_t i = 0; i < this_id->menu_buttons.size(); ++i) {
			loc.type = static_cast<menu_button_type>(i);

			generic_call(make_dereferenced_location(this_id->menu_buttons.data() + i, loc));
		}
	}
};