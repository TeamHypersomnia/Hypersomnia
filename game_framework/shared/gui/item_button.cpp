#include "item_button.h"
#include "gui/stroke.h"

void item_button::draw_triangles(draw_info in) {
	rgba inside_col = cyan;
	rgba border_col = cyan;

	inside_col.a = 20;
	border_col.a = 220;
	
	if (detector.is_hovered || detector.current_appearance == decltype(detector)::appearance::pushed) {
		inside_col.a = 40;
		border_col.a = 255;
	}

	draw_stretched_texture(in, augs::gui::material(assets::texture_id::BLANK, inside_col));
	augs::gui::solid_stroke stroke;
	stroke.set_material(augs::gui::material(assets::texture_id::BLANK, border_col));
	stroke.draw(in.v, *this);
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