#include "slot_button.h"
#include "entity_system/entity.h"

#include "game_framework/shared/inventory_slot.h"
#include "augs/gui/stroke.h"

void slot_button::draw_triangles(draw_info info) {
	auto is_hand_slot = slot_id.is_hand_slot();

	rgba inside_col, border_col;
	
	if (slot_id->for_categorized_items_only) {
		inside_col = orange;
	}
	else
		inside_col = cyan;

	border_col = inside_col;
	inside_col.a = 12*5;
	border_col.a = 255;

	if (detector.current_appearance == augs::gui::appearance_detector::appearance::released) {
		inside_col.a = 4 * 5;
		border_col.a = 220;
	}

	auto inside_tex = slot_id->is_attachment_slot ? assets::texture_id::ATTACHMENT_CIRCLE_FILLED : assets::texture_id::BLANK;
	auto border_tex = slot_id->is_attachment_slot ? assets::texture_id::ATTACHMENT_CIRCLE_BORDER : assets::texture_id::BLANK;

	augs::gui::material inside_mat(inside_tex, inside_col);
	augs::gui::material border_mat(border_tex, border_col);

	if (slot_id->is_attachment_slot) {
		if (slot_id.has_items()) {
			return;
		}
		else {
			draw_centered_texture(info, inside_mat);
			draw_centered_texture(info, border_mat);

			if (slot_id.type == slot_function::PRIMARY_HAND) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::PRIMARY_HAND_ICON, border_col), vec2i(1, 0));
			}

			if (slot_id.type == slot_function::SECONDARY_HAND) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::SECONDARY_HAND_ICON, border_col));
			}

			if (slot_id.type == slot_function::SHOULDER_SLOT) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::SHOULDER_SLOT_ICON, border_col));
			}

			if (slot_id.type == slot_function::TORSO_ARMOR_SLOT) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::ARMOR_SLOT_ICON, border_col));
			}
		}
	}
	else {

	}
}

void slot_button::perform_logic_step(augs::gui::gui_world& gr) {
	vec2i parent_position;
	
	if (slot_id.container_entity->find<components::item>()) {

	}

	auto gridded_offset = user_drag_offset;
	gridded_offset /= 10;
	gridded_offset *= 10;

	rc.set_position(parent_position + slot_relative_pos + gridded_offset);
}

void slot_button::consume_gui_event(event_info info) {
	detector.update_appearance(info);
	
	if (info == rect::gui_event::ldrag) {
		user_drag_offset = (info.owner.state.mouse.pos - slot_relative_pos - info.owner.ldrag_relative_anchor);
	}
}