#pragma once
#include "app_ui_element_location.h"
#include "augs/gui/appearance_detector.h"

#include "augs/padding_byte.h"

class dx_button : public app_ui_rect_node {
public:
	augs::gui::appearance_detector detector;
	bool click_callback_required = false;
	padding_byte pad[3];
	rgba colorize;

	typedef app_ui_rect_node base;
	typedef base::gui_entropy gui_entropy;

	template <class C, class D>
	static void advance_elements(C context, const D& this_id, const gui_entropy& entropies) {
		for (const auto& info : entropies.get_events_for(this_id)) {
			detector.update_appearance(info);
			
			if (info.msg == gui_event::lclick) {
				click_callback_required = true;
			}
		}
	}

	template <class C, class D>
	static void draw(C context, const D& this_id, augs::gui::draw_info in) {
		if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
			return;
		}

		const auto& detector = this_id->detector;
		const auto& rect_world = context.get_rect_world();
		const auto& this_tree_entry = context.get_tree_entry(this_id);


	}
};