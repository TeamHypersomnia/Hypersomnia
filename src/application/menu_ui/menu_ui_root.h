#pragma once
#include "application/menu_ui/menu_ui_element_location.h"
#include "application/menu_ui/option_button.h"
#include "augs/gui/dereferenced_location.h"
#include "augs/misc/enum_array.h"

template <class Enum>
class menu_ui_root : public menu_ui_rect_node<Enum> {
public:
	augs::enum_array<
		option_button<Enum>,
		Enum
	> menu_buttons;

	menu_ui_root(const vec2i screen_size) {
		unset_flag(augs::gui::flag::CLIP);
		set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
		set_flag(augs::gui::flag::DISABLE_HOVERING);

		rc = xywh(0, 0, 0, 0);

		set_screen_size(screen_size);
	}

	void set_screen_size(const vec2i screen_size) {
		set_menu_buttons_positions(screen_size);
	}

	void set_menu_buttons_positions(const vec2i screen_size) {
		set_menu_buttons_sizes(get_max_menu_button_size());

		for (size_t i = menu_buttons.size() - 1; i != size_t(-1); --i) {
			if (i == menu_buttons.size() - 1) {
				menu_buttons[i].rc.set_position(vec2(70.f, screen_size.y - 70.f - menu_buttons[i].rc.h()));
			}
			else {
				menu_buttons[i].rc.set_position(vec2(70.f, menu_buttons[i + 1].rc.t - 22 - menu_buttons[i].rc.h()));
			}
		}
	}

	void set_menu_buttons_sizes(const vec2i size) {
		for (size_t i = 0; i < menu_buttons.size(); ++i) {
			auto this_size = size;

			const auto bbox = menu_buttons[i].corners.cornered_size_to_internal_size(menu_buttons[i].get_target_button_size());

			this_size.x = std::min(bbox.x, this_size.x);
			this_size.y = std::min(bbox.y, this_size.y);

			menu_buttons[i].rc.set_size(menu_buttons[i].corners.internal_size_to_cornered_size(this_size));
		}
	}

	vec2i get_max_menu_button_size() const {
		vec2i s;

		for (size_t i = 0; i < menu_buttons.size(); ++i) {
			const auto bbox = menu_buttons[i].get_target_button_size();

			s.x = std::max(bbox.x, s.x);
			s.y = std::max(bbox.y, s.y);
		}

		return s;
	}

	void set_menu_buttons_colors(const rgba col) {
		for (size_t i = 0; i < menu_buttons.size(); ++i) {
			menu_buttons[i].colorize = col;
		}
	}

	template <class C, class D, class L>
	static void for_each_child(const C context, const D this_id, L generic_call) {
		option_button_in_menu<Enum, option_button<Enum>> loc;

		for (std::size_t i = 0; i < this_id->menu_buttons.size(); ++i) {
			loc.type = static_cast<Enum>(i);

			generic_call(make_dereferenced_location(this_id->menu_buttons.data() + i, loc));
		}
	}

	void draw_background_behind_buttons(
		augs::vertex_triangle_buffer& output,
		vec2 expand = { 14, 10 },
		rgba filling_col = { 0, 0, 0, 180 },
		rgba border_col = slightly_visible_white
	) const {
		ltrb buttons_bg;
		buttons_bg.set_position(menu_buttons.front().rc.left_top());
		buttons_bg.b = menu_buttons.back().rc.b;
		buttons_bg.w(static_cast<float>(get_max_menu_button_size().x));

		augs::draw_rect_with_border(
			output,
			buttons_bg.expand_from_center(expand),
			filling_col,
			border_col
		);
	}
};