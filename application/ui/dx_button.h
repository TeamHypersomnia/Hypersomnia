#pragma once
#include "app_ui_element_location.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/material.h"

#include "augs/padding_byte.h"

#include "game/assets/texture_id.h"

#include "application/ui/button_corners.h"
#include "application/ui/appearing_text.h"

class dx_button : public app_ui_rect_node {
public:
	augs::gui::appearance_detector detector;
	augs::gui::text::fstr caption;
	appearing_text appearing_caption;
	button_corners_info corners;

	rgba colorize;
	bool click_callback_required = false;
	padding_byte pad[3];

	typedef app_ui_rect_node base;
	typedef base::gui_entropy gui_entropy;

	vec2i get_target_button_size() const {
		return corners.internal_size_to_cornered_size(get_text_bbox(appearing_caption.get_total_target_text(), 0)); - vec2i(0, 3);
	}

	void set_appearing_caption(const augs::gui::text::fstr text) {
		appearing_caption.population_interval = 100.f;

		appearing_caption.should_disappear = false;
		appearing_caption.target_text[0] = text;
	}
		
	template <class C, class D>
	static void advance_elements(C context, const D& this_id, const gui_entropy& entropies, const augs::delta) {
		for (const auto& info : entropies.get_events_for(this_id)) {
			this_id->detector.update_appearance(info);
			
			if (info.msg == gui_event::lclick) {
				this_id->click_callback_required = true;
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

		const auto inside_mat = augs::gui::material(assets::texture_id::HOTBAR_BUTTON_INSIDE, this_id->colorize);

		const auto internal_rc = this_id->corners.cornered_rc_to_internal_rc(this_id->rc);

		augs::gui::draw_clipped_rectangle(inside_mat, internal_rc, {}, in.v);
		
		//{
		//	this_id->corners.for_each_button_corner(internal_rc, [this_id, in](const assets::texture_id id, const ltrb drawn_rc) {
		//		augs::gui::draw_clipped_rectangle(augs::gui::material(id, this_id->colorize), drawn_rc, {}, in.v, true);
		//	});
		//}

		this_id->appearing_caption.target_pos = internal_rc.left_top();
		this_id->appearing_caption.draw(in.v);
	}
};