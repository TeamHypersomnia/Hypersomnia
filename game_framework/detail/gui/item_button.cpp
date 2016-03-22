#include "item_button.h"
#include "gui/stroke.h"

#include "entity_system/world.h"
#include "game_framework/systems/gui_system.h"

#include "game_framework/detail/inventory_slot.h"
#include "game_framework/detail/inventory_utils.h"
#include "game_framework/components/gui_element_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/item_component.h"

#include "game_framework/detail/state_for_drawing.h"
#include "game_framework/settings.h"

#include "game_framework/systems/input_system.h"

#include "augs/graphics/renderer.h"
#include "game_framework/resources/manager.h"

#include "augs/stream.h"

#include "pixel_line_connector.h"
#include "grid.h"
#include "log.h"
void item_button::get_member_children(std::vector<augs::gui::rect_id>& children) {
	// children.push_back(&charges_caption);
}

bool item_button::is_being_dragged_or_pending_finish(augs::gui::gui_world& gr) {
	if (is_being_dragged(gr))
		return true;

	auto& gui_intents = gui_element_entity->get_owner_world().get_system<input_system>().gui_item_transfer_intent_player.get_pending_inputs_for_logic();
	
	for (auto& g : gui_intents) {
		if (g.item == item) {
			return true;
		}
	}

	return false;
}

item_button::item_button(rects::xywh<float> rc) : rect(rc) {
	clip = false;
	scrollable = false;
	focusable = false;
}

void item_button::draw_dragged_ghost_inside(draw_info in) {
	draw_proc(in, true, false, false, true);
}

void item_button::draw_complete_with_children(draw_info in) {
	draw_children(in);
	draw_proc(in, true, true, true, false, false, true);
}

void item_button::draw_grid_border_ghost(draw_info in) {
	draw_proc(in, false, true, false, true, true);
}



void item_button::draw_complete_dragged_ghost(draw_info in) {
	auto parent_slot = item->get<components::item>().current_slot;

	auto prev_abs = absolute_xy;
	absolute_xy += in.owner.current_drag_amount;

	draw_dragged_ghost_inside(in);

	absolute_xy = prev_abs + griddify(in.owner.current_drag_amount);
}

rects::ltrb<float> item_button::iterate_children_attachments(bool draw, std::vector<vertex_triangle>* target, augs::rgba border_col) {
	auto item_sprite = item->get<components::sprite>();

	const auto& gui_def = resource_manager.find(item_sprite.tex)->gui_sprite_def;

	item_sprite.flip_horizontally = gui_def.flip_horizontally;
	item_sprite.flip_vertically = gui_def.flip_vertically;
	item_sprite.rotation_offset = gui_def.rotation_offset;

	item_sprite.color.a = border_col.a;

	shared::state_for_drawing_renderable state;
	state.screen_space_mode = true;
	state.overridden_target_buffer = target;
	
	auto expanded_size = rc.get_size() - with_attachments_bbox.get_size();

	state.renderable_transform.pos = get_absolute_xy() - with_attachments_bbox.get_position() + expanded_size/2 + vec2(1, 1);

	rects::ltrb<float> button_bbox = item_sprite.get_aabb(components::transform(), true);

	if (!is_container_open) {
		for_each_descendant(item, [this, draw, &item_sprite, &state, &button_bbox](augs::entity_id desc) {
			if (desc == item)
				return;

			auto parent_slot = desc->get<components::item>().current_slot;

			if (parent_slot.should_item_inside_keep_physical_body(item)) {
				auto attachment_sprite = desc->get<components::sprite>();

				attachment_sprite.flip_horizontally = item_sprite.flip_horizontally;
				attachment_sprite.flip_vertically = item_sprite.flip_vertically;
				attachment_sprite.rotation_offset = item_sprite.rotation_offset;

				attachment_sprite.color.a = item_sprite.color.a;
				shared::state_for_drawing_renderable attachment_state = state;
				auto offset = parent_slot.sum_attachment_offsets_of_parents(desc) - item->get<components::item>().current_slot.sum_attachment_offsets_of_parents(item);
				
				if (attachment_sprite.flip_horizontally) {
					offset.pos.x = -offset.pos.x;
					offset.flip_rotation();
				}

				if (attachment_sprite.flip_vertically) {
					offset.pos.y = -offset.pos.y;
					offset.flip_rotation();
				}

				offset += item_sprite.size / 2;
				offset += -attachment_sprite.size / 2;

				attachment_state.renderable_transform += offset;

				if (draw)
					attachment_sprite.draw(attachment_state);

				rects::ltrb<float> attachment_bbox = attachment_sprite.get_aabb(offset, true);
				button_bbox.contain(attachment_bbox);
			}
		});
	}

	if(draw)
		item_sprite.draw(state);

	return button_bbox;
}

void item_button::draw_proc(draw_info in, bool draw_inside, bool draw_border, bool draw_connector, bool decrease_alpha, bool decrease_border_alpha, bool draw_container_opened_mark) {
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
		inside_col.a = 15;
	}

	if (decrease_border_alpha) {
		border_col = slightly_visible_white;
	}

	if (draw_inside) {
		draw_stretched_texture(in, augs::gui::material(assets::texture_id::BLANK, inside_col));

		iterate_children_attachments(true, &in.v, border_col);

		auto& item_data = item->get<components::item>();

		long double bottom_number_val = -1.f;
		auto* container = item->find<components::container>();
		bool append_x = false;

		auto label_color = border_col;

		if (item_data.charges > 1) {
			bottom_number_val = item_data.charges;
			append_x = true;
		}
		else if (DRAW_FREE_SPACE_INSIDE_CONTAINER_ICONS && item[slot_function::ITEM_DEPOSIT].alive()) {
			bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_free_space_with_parent_containers() / long double(SPACE_ATOMS_PER_UNIT);

			if (item[slot_function::ITEM_DEPOSIT]->for_categorized_items_only)
				label_color.rgb() = pink.rgb();
			else
				label_color.rgb() = cyan.rgb();
		}

		if (bottom_number_val > -1.f) {
			std::wstring label_wstr;
			
			if (append_x) {
				label_wstr = L'x';
				label_wstr += augs::to_wstring(bottom_number_val);
			}
			else
				label_wstr = augs::to_wstring(bottom_number_val, 2);

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

	if (draw_container_opened_mark) {
		if (item->find<components::container>()) {
			components::sprite container_status_sprite;
			if(is_container_open)
				container_status_sprite.set(assets::CONTAINER_OPEN_ICON, border_col);
			else
				container_status_sprite.set(assets::CONTAINER_CLOSED_ICON, border_col);

			shared::state_for_drawing_renderable state;
			state.screen_space_mode = true;
			state.overridden_target_buffer = &in.v;
			state.renderable_transform.pos.set(get_rect_absolute().r - container_status_sprite.size.x + 2, get_rect_absolute().t + 1
				//- container_status_sprite.size.y + 2
				);
			container_status_sprite.draw(state);
		}
	}
}

bool item_button::is_inventory_root() {
	return item == gui_element_entity;
}

void item_button::perform_logic_step(augs::gui::gui_world& gr) {
	enable_drawing_of_children = is_container_open && !is_being_dragged_or_pending_finish(gr);
	disable_hovering = is_being_dragged_or_pending_finish(gr);
	rect::perform_logic_step(gr);

	if (is_inventory_root())
		return;

	vec2i parent_position;

	auto* sprite = item->find<components::sprite>();
	
	if (sprite) {
		with_attachments_bbox = iterate_children_attachments();
		vec2i rounded_size = with_attachments_bbox.get_size();
		rounded_size += 22;
		rounded_size += resource_manager.find(sprite->tex)->gui_sprite_def.gui_bbox_expander;
		rounded_size /= 11;
		rounded_size *= 11;
		//rounded_size.x = std::max(rounded_size.x, 33);
		//rounded_size.y = std::max(rounded_size.y, 33);
		rc.set_size(rounded_size);
	}

	auto parent_slot = item->get<components::item>().current_slot;
	
	if (parent_slot->always_allow_exactly_one_item) {
		rc.set_position(get_meta(parent_slot).rc.get_position());
	}
	else {
		rc.set_position(drag_offset_in_item_deposit);
	}
}
int dragcnt = 0;
void item_button::consume_gui_event(event_info info) {
	if (is_inventory_root())
		return;

	detector.update_appearance(info);
	auto parent_slot = item->get<components::item>().current_slot;

	if (info == rect::gui_event::ldrag) {
		if (!started_drag) {
			started_drag = true;

			if (parent_slot->always_allow_exactly_one_item)
				if (get_meta(parent_slot).get_rect_absolute().hover(info.owner.state.mouse.pos)) {
					get_meta(parent_slot).houted_after_drag_started = false;
				}
		}
	}

	if (info == rect::gui_event::rclick) {
		is_container_open = !is_container_open;
	}

	if (info == rect::gui_event::lfinisheddrag) {
		dragcnt++;
		started_drag = false;

		auto& gui = gui_element_entity->get_owner_world().get_system<gui_system>();
		auto& drag_result = gui.prepare_drag_and_drop_result();

		if (drag_result.possible_target_hovered && drag_result.will_drop_be_successful()) {
			gui.parent_world.post_message(drag_result.intent);
			LOG("finished drag");
		}
		else if (!drag_result.possible_target_hovered) {
			vec2i griddified = griddify(info.owner.current_drag_amount);

			if (parent_slot->always_allow_exactly_one_item) {
				get_meta(parent_slot).user_drag_offset += griddified;
				get_meta(parent_slot).houted_after_drag_started = true;
			}
			else {
				drag_offset_in_item_deposit += griddified;
			}
		}
	}

	// if(being_dragged && inf == rect::gui_event::lup)
}

void item_button::draw_triangles(draw_info in) {
	if (is_inventory_root()) {
		draw_children(in);
		return;
	}

	if (!is_being_dragged_or_pending_finish(in.owner)) {
		draw_complete_with_children(in);
		if (dragcnt == 2 && item->get<components::item>().current_slot.alive() && item->get<components::item>().current_slot.type == slot_function::SECONDARY_HAND) {
			LOG("in secondary");
		}
	}
}

item_button& get_meta(augs::entity_id id) {
	return get_owning_transfer_capability(id)->get<components::gui_element>().item_metadata[id];
}
