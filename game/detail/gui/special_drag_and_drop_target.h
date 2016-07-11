#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "special_controls.h"

class gui_system;
struct special_drag_and_drop_target : augs::gui::rect {
	special_control type;

	special_drag_and_drop_target(augs::gui::material mat);

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
	void perform_logic_step(augs::gui::rect_world&) final;
};
