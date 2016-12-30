#pragma once
#include "application/ui/app_ui_element_location.h"
#include "application/ui/dx_button.h"
#include "application/menu_button_type.h"
#include "augs/gui/dereferenced_location.h"

class app_ui_root : public app_ui_rect_node {
public:
	app_ui_root(const vec2i screen_size);

	std::array<dx_button, static_cast<size_t>(menu_button_type::COUNT)> menu_buttons;

	void set_menu_buttons_sizes(const vec2i size);
	void set_menu_buttons_colors(const rgba col);

	vec2i get_max_menu_button_size() const;

	template <class C, class D, class L>
	static void for_each_child(C context, const D& this_id, L generic_call) {
		// however it should be a container so we call the callback on the element's children
		// i.e. the player has a gui element component and container component but not an item component.
		dx_button_in_menu loc;

		for (size_t i = 0; i < this_id->menu_buttons.size(); ++i) {
			loc.type = static_cast<menu_button_type>(i);

			generic_call(make_dereferenced_location(this_id->menu_buttons.data() + i, loc));
		}
	}
};