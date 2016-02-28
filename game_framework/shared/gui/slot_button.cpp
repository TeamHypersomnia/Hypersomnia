#include "slot_button.h"

#include "game_framework/shared/inventory_slot.h"
#include "augs/gui/stroke.h"

void slot_button::draw_triangles(draw_info info) {
	auto is_hand_slot = slot_id.is_hand_slot();

	rgba inside_attachment_col = orange;
	inside_attachment_col.a = 12;

	rgba attachment_border_col = orange;
	attachment_border_col.a = 255;

	rgba inside_deposit_col = cyan;
	inside_attachment_col.a = 12;

	rgba deposit_border_col = cyan;
	attachment_border_col.a = 255;

	if (is_hand_slot) {
		inside_attachment_col = cyan;
		inside_attachment_col.a = 12;
		attachment_border_col = cyan;
		attachment_border_col.a = 255;
	}

	augs::gui::material inside_deposit_mat(assets::texture_id::BLANK, inside_deposit_col);
	augs::gui::material deposit_border_mat(assets::texture_id::BLANK, deposit_border_col);
	augs::gui::material inside_attachment_mat(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, inside_attachment_col);
	augs::gui::material attachment_border_mat(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, attachment_border_col);

	if (slot_id->is_attachment_slot) {
		if (slot_id.has_items()) {
			return;
		}
		else {
			draw_centered_texture(info, inside_attachment_mat);
			draw_centered_texture(info, attachment_border_mat);

			if (slot_id.type == slot_function::PRIMARY_HAND) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::PRIMARY_HAND_ICON, deposit_border_col), vec2i(1, 0));
			}

			if (slot_id.type == slot_function::SECONDARY_HAND) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::SECONDARY_HAND_ICON, deposit_border_col));
			}

			if (slot_id.type == slot_function::SHOULDER_SLOT) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::SHOULDER_SLOT_ICON, attachment_border_col));
			}

			if (slot_id.type == slot_function::TORSO_ARMOR_SLOT) {
				draw_centered_texture(info, augs::gui::material(assets::texture_id::ARMOR_SLOT_ICON, attachment_border_col));
			}

			//augs::gui::solid_stroke border(1, attachment_border_mat);
			//border.draw(info.v, *this);
		}
	}
	else {

	}
}

void slot_button::consume_gui_event(event_info info) {
	detector.update_appearance(info);
}