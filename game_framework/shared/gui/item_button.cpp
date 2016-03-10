#include "item_button.h"
#include "gui/stroke.h"

#include "game_framework/shared/inventory_slot.h"
#include "game_framework/shared/inventory_utils.h"
#include "game_framework/components/gui_element_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/item_component.h"

#include "game_framework/shared/state_for_drawing.h"
#include "game_framework/settings.h"

#include "augs/graphics/renderer.h"
#include "game_framework/resources/manager.h"

#include "augs/stream.h"

#include "pixel_line_connector.h"
#include "grid.h"

void item_button::get_member_children(std::vector<augs::gui::rect_id>& children) {
	// children.push_back(&charges_caption);
}

item_button::item_button(rects::xywh<float> rc) : rect(rc) {
	clip = false;
	scrollable = false;
	focusable = false;
}

void item_button::draw_triangles(draw_info in) {
	if (is_inventory_root()) {
		draw_children(in);
		return;
	}

	if (is_being_dragged(in.owner)) {
		auto parent_slot = item->get<components::item>().current_slot;

		auto prev_abs = absolute_xy;
		absolute_xy += in.owner.current_drag_amount;

		draw_proc(in, true, false, false, false);

		absolute_xy = prev_abs + griddify(in.owner.current_drag_amount);

		draw_proc(in, false, true, false, true, true);
	}
	else {
		draw_children(in);
		draw_proc(in, true, true, true, false);
	}
}

void item_button::draw_proc(draw_info in, bool draw_inside, bool draw_border, bool draw_connector, bool decrease_alpha, bool decrease_border_alpha) {
	if (is_inventory_root())
		return;
	
	auto parent_slot = item->get<components::item>().current_slot;

	rgba inside_col = cyan;
	rgba border_col = cyan;

	if (parent_slot->for_categorized_items_only) {
		border_col = pink;
		inside_col = violet;
	}

	inside_col.a = 20;
	border_col.a = 190;

	if (detector.is_hovered) {
		inside_col.a = 30;
		border_col.a = 220;
	}

	if (detector.current_appearance == decltype(detector)::appearance::pushed) {
		inside_col.a = 60;
		border_col.a = 255;
	}

	if (decrease_alpha) {
		inside_col.a -= 10;
	}

	if (decrease_border_alpha) {
		border_col = white;
		border_col.a -= 220;
	}

	if (draw_inside) {
		draw_stretched_texture(in, augs::gui::material(assets::texture_id::BLANK, inside_col));

		auto* sprite = item->find<components::sprite>();

		if (sprite) {
			auto transparent_sprite = *sprite;
			const auto& gui_def = resource_manager.find(sprite->tex)->gui_sprite_def;

			transparent_sprite.flip_horizontally = gui_def.flip_horizontally;
			transparent_sprite.flip_vertically = gui_def.flip_vertically;
			transparent_sprite.rotation_offset = gui_def.rotation_offset;

			transparent_sprite.color.a = border_col.a;

			shared::state_for_drawing_renderable state;
			state.screen_space_mode = true;
			state.overridden_target_buffer = &in.v;
			state.renderable_transform.pos = get_absolute_xy() + rc.get_size() / 2 - sprite->size / 2;
			transparent_sprite.draw(state);
		}

		auto& item_data = item->get<components::item>();

		float bottom_number_val = -1.f;
		auto* container = item->find<components::container>();
		bool append_x = false;

		auto label_color = border_col;

		if (item_data.charges > 1) {
			bottom_number_val = item_data.charges;
			append_x = true;
		}
		else if (DRAW_FREE_SPACE_INSIDE_CONTAINER_ICONS && item[slot_function::ITEM_DEPOSIT].alive()) {
			bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_free_space_with_parent_containers();

			if (item[slot_function::ITEM_DEPOSIT]->for_categorized_items_only)
				label_color.rgb() = pink.rgb();
			else
				label_color.rgb() = cyan.rgb();
		}

		if (bottom_number_val > -1.f) {
			auto label_wstr = augs::to_wstring(bottom_number_val);
			if (append_x) label_wstr = L'x' + label_wstr;
			// else label_wstr = L'{' + label_wstr + L'}';

			auto bottom_number = augs::gui::text::format(label_wstr, augs::gui::text::style(assets::font_id::GUI_FONT, label_color));

			charges_caption.set_text(bottom_number);
			charges_caption.bottom_right(get_rect_absolute());
			charges_caption.draw(in);
		}
	}

	if (draw_border) {
		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material(assets::texture_id::BLANK, border_col));
		stroke.draw(in.v, *this);
	}

	if (draw_connector && get_meta(parent_slot).gui_element_entity != parent_slot.container_entity) {
		draw_pixel_line_connector(get_rect_absolute(), get_meta(parent_slot.container_entity).get_rect_absolute(), in, border_col);
	}
}

bool item_button::is_inventory_root() {
	return item == gui_element_entity;
}

void item_button::perform_logic_step(augs::gui::gui_world& gr) {
	enable_drawing_of_children = !is_being_dragged(gr);
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
		rc.set_position(drag_offset_in_item_deposit);
	}
}

void item_button::consume_gui_event(event_info info) {
	if (is_inventory_root())
		return;

	detector.update_appearance(info);
	auto parent_slot = item->get<components::item>().current_slot;

	if (info == rect::gui_event::lfinisheddrag) {
		vec2i griddified = griddify(info.owner.current_drag_amount);

		if (parent_slot->is_attachment_slot)
			get_meta(parent_slot).user_drag_offset += griddified;
		else {
			drag_offset_in_item_deposit += griddified;
		}
	}

	// if(being_dragged && inf == rect::gui_event::lup)
}


item_button& get_meta(augs::entity_id id) {
	return get_owning_transfer_capability(id)->get<components::gui_element>().item_metadata[id];
}
