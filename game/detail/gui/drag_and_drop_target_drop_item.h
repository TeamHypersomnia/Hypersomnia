#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	typedef drag_and_drop_target_drop_item_location location;

	drag_and_drop_target_drop_item(const augs::gui::material new_mat);

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	template<class C, class gui_element_id>
	static void draw(C context, const gui_element_id& this_id, augs::gui::draw_info info) {
		auto mat_coloured = this_id->mat;

		if (this_id->detector.is_hovered)
			mat_coloured.color.a = 255;
		else
			mat_coloured.color.a = 120;

		draw_centered_texture(context, this_id, info, mat_coloured);
	}

	template<class C, class gui_element_id>
	static void consume_gui_event(C context, const gui_element_id& this_id, const augs::gui::event_info info) {
		this_id->detector.update_appearance(info);
	}

	template<class C, class gui_element_id>
	static void perform_logic_step(C context, const gui_element_id& this_id, const fixed_delta& dt) {
		auto& world = context.get_rect_world();
		auto dragged_item = world.rect_held_by_lmb;

		if (context.alive(dragged_item) && world.held_rect_is_dragged) {
			this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
		}
		else {
			this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
		}

		this_id->rc.set_position(context.get_gui_element_component().get_initial_position_for(*this_id) - vec2(20, 20));
		this_id->rc.set_size((*this_id->mat.tex).get_size() + vec2(40, 40));
	}
};