#include "item_button.h"
#include "gui/stroke.h"

#include "game_framework/shared/inventory_slot.h"
#include "game_framework/shared/inventory_utils.h"
#include "game_framework/components/gui_element_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/item_component.h"

#include "game_framework/shared/state_for_drawing.h"

#include "augs/graphics/renderer.h"
#include "game_framework/resources/manager.h"

#include "augs/stream.h"

#include "pixel_line_connector.h"

void item_button::get_member_children(std::vector<augs::gui::rect_id>& children) {
	// children.push_back(&charges_caption);
}

item_button::item_button(rects::xywh<float> rc) : rect(rc) {
	clip = false;
	scrollable = false;
	focusable = false;
}

void item_button::draw_triangles(draw_info in) {
	draw_children(in);

	draw_proc(in, false);

	if (is_being_dragged(in.owner)) {
		auto old_rc = rc;
		rc = get_rect_absolute();

		auto parent_slot = item->get<components::item>().current_slot;

		//if (parent_slot->is_attachment_slot) {
			//auto gridded_absolute_pos =	get_meta(parent_slot).get_rect_absolute().get_position() + user_drag_offset;
			// gridded_absolute_pos /= 11;
			// gridded_absolute_pos *= 11;
			absolute_xy += in.owner.current_drag_amount;
		//}

		draw_proc(in, true);
	
		rc = old_rc;
	}
}

void item_button::draw_proc(draw_info in, bool dragged_ghost) {
	if (is_inventory_root())
		return;

	rgba inside_col = cyan;
	rgba border_col = cyan;

	inside_col.a = 20;
	border_col.a = 220;

	if (detector.is_hovered || detector.current_appearance == decltype(detector)::appearance::pushed) {
		inside_col.a = 40;
		border_col.a = 255;
	}

	if (dragged_ghost) {
		inside_col.a -= 10;
	}

	bool draw_border = !dragged_ghost;
	bool draw_inside = dragged_ghost || !is_being_dragged(in.owner);

	if(draw_inside)
		draw_stretched_texture(in, augs::gui::material(assets::texture_id::BLANK, inside_col));

	if (draw_border) {
		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material(assets::texture_id::BLANK, border_col));
		stroke.draw(in.v, *this);
	}

	auto* sprite = item->find<components::sprite>();

	if (draw_inside && sprite) {
		auto transparent_sprite = *sprite;
		const auto& gui_def = resource_manager.find(sprite->tex)->gui_sprite_def;

		transparent_sprite.flip_horizontally = gui_def.flip_horizontally;
		transparent_sprite.flip_vertically = gui_def.flip_vertically;
		transparent_sprite.rotation_offset = gui_def.rotation_offset;

		transparent_sprite.color.a = border_col.a;

		shared::state_for_drawing_renderable state;
		state.screen_space_mode = true;
		state.overridden_target_buffer = &in.v;
		state.renderable_transform.pos = get_absolute_xy() + rc.get_size()/2 - sprite->size/2;
		transparent_sprite.draw(state);
	}

	auto& item_data = item->get<components::item>();

	if (draw_inside && item_data.charges > 1) {
		auto charges_text = augs::gui::text::format(augs::to_wstring(item_data.charges)
			, augs::gui::text::style(assets::font_id::GUI_FONT, border_col));

		charges_caption.set_text(charges_text);
		charges_caption.bottom_right(get_rect_absolute());
		charges_caption.draw(in);
	}

	auto parent_slot = item->get<components::item>().current_slot;

	if (!dragged_ghost && get_meta(parent_slot).gui_element_entity != parent_slot.container_entity) {
		draw_pixel_line_connector(get_rect_absolute(), get_meta(parent_slot.container_entity).get_rect_absolute(), in, border_col);
	}
}

bool item_button::is_inventory_root() {
	return item == gui_element_entity;
}

void item_button::perform_logic_step(augs::gui::gui_world& gr) {
	rect::perform_logic_step(gr);
	vec2i parent_position;

	auto* sprite = item->find<components::sprite>();
	
	if (sprite) {
		vec2i rounded_size = sprite->size;
		rounded_size += 22;
		rounded_size /= 11;
		rounded_size *= 11;
		rc.set_size(rounded_size);
	}

	if (is_inventory_root())
		return;

	auto parent_slot = item->get<components::item>().current_slot;
	
	if (parent_slot->is_attachment_slot) {
		rc.set_position(get_meta(parent_slot).rc.get_position());
	}
	else {
		auto gridded_absolute_pos = drag_offset_in_item_deposit;
		gridded_absolute_pos /= 11;
		gridded_absolute_pos *= 11;
		rc.set_position(gridded_absolute_pos);
	}
}

void item_button::consume_gui_event(event_info info) {
	if (is_inventory_root())
		return;

	detector.update_appearance(info);
	auto parent_slot = item->get<components::item>().current_slot;

	if (info == rect::gui_event::lfinisheddrag) {
		if (parent_slot->is_attachment_slot)
			get_meta(parent_slot).user_drag_offset += info.owner.current_drag_amount;
		else
			drag_offset_in_item_deposit += info.owner.current_drag_amount;
	}

	// if(being_dragged && inf == rect::gui_event::lup)
}


item_button& get_meta(augs::entity_id id) {
	return get_owning_transfer_capability(id)->get<components::gui_element>().item_metadata[id];
}
