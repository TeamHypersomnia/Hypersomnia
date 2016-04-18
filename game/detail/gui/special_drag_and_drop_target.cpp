#include "special_drag_and_drop_target.h"
#include "item_button.h"
#include "../../systems/gui_system.h"

special_drag_and_drop_target::special_drag_and_drop_target(augs::gui::material mat) : mat(mat) {
	clip = false;
	enable_drawing = false;
}

void special_drag_and_drop_target::draw_triangles(draw_info info) {
	auto mat_coloured = mat;

	if (detector.is_hovered)
		mat_coloured.color.a = 255;
	else
		mat_coloured.color.a = 120;

	draw_centered_texture(info, mat_coloured);
}

void special_drag_and_drop_target::consume_gui_event(event_info info) {
	detector.update_appearance(info);
}

void special_drag_and_drop_target::perform_logic_step(augs::gui::gui_world& gui) {
	game_gui_world& game_gui = (game_gui_world&)gui;

	auto dragged_item = dynamic_cast<item_button*>(gui.rect_held_by_lmb) ;

	enable_drawing = dragged_item != nullptr && gui.held_rect_is_dragged;
	rc.set_position(game_gui.gui_system->get_initial_position_for_special_control(type) - vec2(20, 20));
	rc.set_size((*mat.tex).get_size() + vec2(40, 40));
}