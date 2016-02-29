#include "item_button.h"

void item_button::draw_triangles(draw_info info) {

}

void item_button::perform_logic_step(augs::gui::gui_world& gr) {
	vec2i parent_position;

	//if (slot_id.container_entity->find<components::item>()) {
	//
	//}
	//
	//auto gridded_offset = user_drag_offset;
	//gridded_offset /= 10;
	//gridded_offset *= 10;
	//
	//rc.set_position(parent_position + slot_relative_pos + gridded_offset);
}

void item_button::consume_gui_event(event_info info) {
	detector.update_appearance(info);
}