#pragma once
#include <cstddef>
#include "application/gui/menu/menu_element_location.h"
#include "application/gui/menu/option_button.h"
#include "application/gui/main_menu_button_type.h"
#include "augs/gui/dereferenced_location.h"
#include "augs/misc/enum/enum_array.h"

template <class Enum>
class menu_root : public menu_rect_node<Enum> {
public:
	using base = menu_rect_node<Enum>;
	using base::set_flag;
	using base::unset_flag;
	using base::rc;

	augs::enum_array<
		option_button<Enum>,
		Enum
	> buttons;

	float scale = 1.0f;

	menu_root() {
		unset_flag(augs::gui::flag::CLIP);
		set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
		set_flag(augs::gui::flag::DISABLE_HOVERING);

		rc = xywh(0, 0, 0, 0);
	}

	template <class M>
	void set_menu_buttons_positions(
		const M& manager, 
		const augs::baked_font& gui_font, 
		const vec2i screen_size
	) {
		set_menu_buttons_sizes(manager, gui_font, get_max_menu_button_size(manager, gui_font));

#if WEB_LOWEND
		const auto menu_bottom_margin_v = 30.0f;
#else
		const auto menu_bottom_margin_v = 70.0f;
#endif

		for (size_t i = buttons.size() - 1; i != size_t(-1); --i) {
			if (buttons[i].hide) {
				if (i < buttons.size() - 1) {
					buttons[i].rc = buttons[i + 1].rc;
				}
				else {
					buttons[i].rc = {};
				}

				continue;
			}

			if (i == buttons.size() - 1) {
				buttons[i].rc.set_position(vec2(70.f, screen_size.y - menu_bottom_margin_v - buttons[i].rc.h()));
			}
			else {
#if WEB_LOWEND
				const auto padding = 8;
#else
				const auto padding = 18;
#endif

				buttons[i].rc.set_position(vec2(70.f, buttons[i + 1].rc.t - (padding * scale) - buttons[i].rc.h()));
			}
		}

		auto& s_rc = buttons[int(Enum::STEAM)].rc;
		auto& d_rc = buttons[int(Enum::DISCORD)].rc;
		auto& g_rc = buttons[int(Enum::GITHUB)].rc;


		auto menu_b = 104 + 70;

		if constexpr(std::is_same_v<Enum, ingame_menu_button_type>) {
			menu_b = 20;
		}
		else {
			s_rc.l += 415;
			s_rc.r += 415;
		}

		auto t = menu_b + 10;
		auto b = t + g_rc.h();

		d_rc = s_rc;
		d_rc.l = s_rc.r + 10;
		d_rc.w(s_rc.w());

		g_rc = d_rc;
		g_rc.l = d_rc.r + 10;
		g_rc.w(d_rc.w());

		g_rc.t = s_rc.t = d_rc.t = t;
		g_rc.b = s_rc.b = d_rc.b = b;
	}

	template <class M>
	void set_menu_buttons_sizes(
		const M& manager, 
		const augs::baked_font& gui_font,
		const vec2i size
	) {
		for (size_t i = 0; i < buttons.size(); ++i) {
			if (buttons[i].hide) {
				continue;
			}

			if (buttons[i].special_image.has_value()) {
				buttons[i].rc.set_size(manager.at(*buttons[i].special_image).get_original_size());
				continue;
			}

			auto this_size = size;

			const auto bbox = buttons[i].corners.cornered_size_to_internal_size(manager, buttons[i].get_target_button_size(manager, gui_font));

			this_size.x = std::min(bbox.x, this_size.x);
			this_size.y = std::min(bbox.y, this_size.y);

			buttons[i].rc.set_size(buttons[i].corners.internal_size_to_cornered_size(manager, this_size));
		}
	}

	template <class M>
	vec2i get_max_menu_button_size(
		const M& manager, 
		const augs::baked_font& gui_font
	) const {
		vec2i s;

		for (size_t i = 0; i < buttons.size(); ++i) {
			if (buttons[i].hide) {
				continue;
			}

			const auto bbox = buttons[i].get_target_button_size(manager, gui_font);

			s.x = std::max(bbox.x, s.x);
			s.y = std::max(bbox.y, s.y);
		}

		return s;
	}

	void set_menu_buttons_colors(const rgba col) {
		for (size_t i = 0; i < buttons.size(); ++i) {
			buttons[i].colorize = col;
		}
	}

	template <class C, class gui_element_id>
	static void rebuild_layouts(
		const C context,
		const gui_element_id this_id
	) {
		this_id->set_menu_buttons_positions(context.get_necessary_images(), context.get_gui_font(), context.get_screen_size());

		menu_rect_node<Enum>::rebuild_layouts(context, this_id);
	}

	template <class C, class D, class L>
	static void for_each_child(const C&, const D this_id, L generic_call) {
		option_button_in_menu<Enum, option_button<Enum>> loc;

		for (std::size_t i = 0; i < this_id->buttons.size(); ++i) {
			if (this_id->buttons[i].hide) {
				continue;
			}

			loc.type = static_cast<Enum>(i);

			generic_call(make_dereferenced_location(this_id->buttons.data() + i, loc));
		}
	}

	template <class C>
	auto get_menu_ltrb(const C context) const {
		vec2 expand = { 14, 10 };

		ltrb buttons_bg;
		buttons_bg.set_position(buttons.front().rc.left_top());
		buttons_bg.w(static_cast<float>(get_max_menu_button_size(context.get_necessary_images(), context.get_gui_font()).x));
		buttons_bg.b = buttons.back().rc.b;

		return buttons_bg.expand_from_center(expand);
	}

	template <class C>
	void draw_background_behind_buttons(
		const C context,
		vec2 expand = { 14, 10 },
		rgba filling_col = { 0, 0, 0, 180 },
		rgba border_col = slightly_visible_white
	) const {
		ltrb buttons_bg;
		buttons_bg.set_position(buttons.back().rc.left_top());
		buttons_bg.b = buttons.back().rc.b;
		buttons_bg.w(static_cast<float>(get_max_menu_button_size(context.get_necessary_images(), context.get_gui_font()).x));
#if WEB_LOWEND
		buttons_bg.r += buttons_bg.l + 18;
#else
		buttons_bg.r += buttons_bg.l + 8;
#endif
		buttons_bg.l = 0;
		buttons_bg.t = 0;
		buttons_bg.b = (context.get_screen_size().y);

		border_col.a = 35;

		context.get_output().aabb_with_border(
			buttons_bg.expand_from_center(expand),
			filling_col,
			border_col
		);
	}
};