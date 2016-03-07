#include "slot_button.h"
#include "entity_system/entity.h"

#include "augs/gui/stroke.h"
#include "item_button.h"
#include "game_framework/shared/inventory_slot.h"
#include "game_framework/shared/inventory_utils.h"
#include "game_framework/components/gui_element_component.h"

#include "augs/stream.h"
#include "pixel_line_connector.h"

slot_button::slot_button() {
	clip = false;
}

void slot_button::get_member_children(std::vector<augs::gui::rect_id>& children) {

}

void slot_button::draw_triangles(draw_info info) {
	auto is_hand_slot = slot_id.is_hand_slot();

	rgba inside_col, border_col;
	
	if (slot_id->for_categorized_items_only) {
		inside_col = orange;
	}
	else
		inside_col = cyan;

	border_col = inside_col;
	inside_col.a = 4 * 5;
	border_col.a = 220;

	if (detector.is_hovered || detector.current_appearance == decltype(detector)::appearance::pushed) {
		inside_col.a = 12 * 5;
		border_col.a = 255;
	}

	auto inside_tex = assets::texture_id::ATTACHMENT_CIRCLE_FILLED;
	auto border_tex = assets::texture_id::ATTACHMENT_CIRCLE_BORDER;

	augs::gui::material inside_mat(inside_tex, inside_col);
	augs::gui::material border_mat(border_tex, border_col);

	if (slot_id->is_attachment_slot) {
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

		if (slot_id.type == slot_function::GUN_CHAMBER) {
			draw_centered_texture(info, augs::gui::material(assets::texture_id::CHAMBER_SLOT_ICON, border_col));
		}

		if (slot_id.type == slot_function::GUN_DETACHABLE_MAGAZINE) {
			draw_centered_texture(info, augs::gui::material(assets::texture_id::DETACHABLE_MAGAZINE_ICON, border_col));
		}
	}
	else {
		draw_centered_texture(info, inside_mat);
		draw_centered_texture(info, border_mat);

		auto space_available_text = augs::gui::text::format(augs::to_wstring(slot_id.calculate_free_space_with_parent_containers(), 2)
			, augs::gui::text::style(assets::font_id::GUI_FONT, border_col));

		space_caption.set_text(space_available_text);
		space_caption.center(get_rect_absolute());
		space_caption.draw(info);

		draw_children(info);
	}

	if (gui_element_entity != slot_id.container_entity) {
		draw_pixel_line_connector(get_rect_absolute(), get_meta(slot_id.container_entity).get_rect_absolute(), info, border_col);
	}
}

void slot_button::perform_logic_step(augs::gui::gui_world& gr) {
	rect::perform_logic_step(gr);

	if (slot_id->is_attachment_slot) {
		enable_drawing = true;

		if (slot_id.has_items()) {
			if (get_meta(slot_id->items_inside[0]).is_being_dragged(gr))
				enable_drawing = true;
			else
				enable_drawing = false;
		}
	}

	auto gridded_absolute_pos = slot_relative_pos + user_drag_offset;
	
	if (is_being_dragged(gr))
		gridded_absolute_pos += gr.current_drag_amount;

	gridded_absolute_pos /= 11;
	gridded_absolute_pos *= 11;
	rc.set_position(gridded_absolute_pos);
}

void slot_button::consume_gui_event(event_info info) {
	detector.update_appearance(info);
	
	if (info == rect::gui_event::lfinisheddrag)
		user_drag_offset += info.owner.current_drag_amount;
}

slot_button& get_meta(inventory_slot_id id) {
	return get_owning_transfer_capability(id.container_entity)->get<components::gui_element>().slot_metadata[id];
}
