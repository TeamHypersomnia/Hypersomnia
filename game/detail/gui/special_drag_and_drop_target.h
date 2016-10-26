#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"
#include "special_controls.h"

#include "game/detail/gui/gui_element_location.h"

struct special_drag_and_drop_target : game_gui_rect_leaf<special_drag_and_drop_target> {
	special_drag_and_drop_target(const augs::gui::material new_mat);

	special_control type;

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	template<class C>
	void draw(C context, augs::gui::draw_info info) const {
		auto mat_coloured = mat;

		if (detector.is_hovered)
			mat_coloured.color.a = 255;
		else
			mat_coloured.color.a = 120;

		draw_centered_texture(context, info, mat_coloured);
	}

	template<class C>
	void consume_gui_event(C context, const augs::gui::event_info) {
		detector.update_appearance(info);
	}

	template<class C>
	void perform_logic_step(C context) {
		auto& world = context.get_rect_world();
		auto dragged_item = world.rect_held_by_lmb;

		enable_drawing = context.alive(dragged_item) && world.held_rect_is_dragged;
		rc.set_position(context.get_gui_element_component().get_initial_position_for_special_control(type) - vec2(20, 20));
		rc.set_size((*mat.tex).get_size() + vec2(40, 40));
	}
};