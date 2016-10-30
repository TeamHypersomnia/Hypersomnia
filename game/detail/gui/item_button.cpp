#include "item_button.h"
#include "pixel_line_connector.h"
#include "grid.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/renderer.h"
#include "augs/templates.h"

#include "augs/gui/stroke.h"

#include "game/globals/item_category.h"
#include "game/detail/state_for_drawing.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_utils.h"
#include "game/components/gui_element_component.h"
#include "game/components/sprite_component.h"
#include "game/components/item_component.h"
#include "game/systems/gui_system.h"
#include "game/systems/input_system.h"
#include "game/resources/manager.h"

#include "augs/ensure.h"

bool item_button::is_being_wholely_dragged_or_pending_finish(const const_dispatcher_context& context, const gui_element_location& this_id) {
	const auto& rect_world = context.get_rect_world();
	const auto& element = context.get_gui_element_component();
	const auto& gui_element_entity = context.get_gui_element_entity();

	this_id.get<item_button_for_item_component_location>()

	if (rect_world.is_being_dragged(this_id)) {
		const bool is_drag_partial = element.dragged_charges < item.get<components::item>().charges;
		return !is_drag_partial;
	}

	return false;
}

item_button::item_button(rects::xywh<float> rc) : rect(rc) {
	clip = false;
	scrollable = false;
	focusable = false;
}

void item_button::draw_dragged_ghost_inside(draw_info in) {
	draw_proc(in, true, false, false, true, false, false, false);
}

void item_button::draw_complete_with_children(draw_info in) {
	draw_children(in);
	draw_proc(in, true, true, true, false, false, true);
}

void item_button::draw_grid_border_ghost(draw_info in) {
	auto prev_abs = absolute_xy;
	absolute_xy = prev_abs + griddify(in.owner.current_drag_amount);
	draw_proc(in, false, true, false, true, true);
	absolute_xy = prev_abs;
}

void item_button::draw_complete_dragged_ghost(draw_info in) {
	auto parent_slot = item->get<components::item>().current_slot;
	ensure(parent_slot.alive());
	auto prev_abs = absolute_xy;
	absolute_xy += in.owner.current_drag_amount;
	draw_dragged_ghost_inside(in);

	absolute_xy = prev_abs;
}

void item_button::draw_proc(draw_info in, bool draw_inside, bool draw_border, bool draw_connector, bool decrease_alpha, bool decrease_border_alpha, bool draw_container_opened_mark, bool draw_charges) {
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

		if (draw_charges) {
			auto& item_data = item->get<components::item>();

			int considered_charges = item_data.charges;

			if (is_being_dragged(in.owner)) {
				considered_charges = item_data.charges - ((game_gui_world&)in.owner).dragged_charges;
			}

			long double bottom_number_val = -1.f;
			auto* container = item->find<components::container>();
			bool printing_charge_count = false;
			bool trim_zero = false;

			auto label_color = border_col;

			if (considered_charges > 1) {
				bottom_number_val = considered_charges;
				printing_charge_count = true;
			}
			else if (item->get_owner_world().systems.get<gui_system>().draw_free_space_inside_container_icons && item[slot_function::ITEM_DEPOSIT].alive()) {
				if (item->get<components::item>().categories_for_slot_compatibility & item_category::MAGAZINE) {
					if (!is_container_open) {
						printing_charge_count = true;
					}
				}

				if (printing_charge_count) {
					bottom_number_val = count_charges_in_deposit(item);
				}
				else {
					bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_free_space_with_parent_containers() / long double(SPACE_ATOMS_PER_UNIT);

					if (bottom_number_val < 1.0 && bottom_number_val > 0.0) {
						trim_zero = true;
					}

					label_color.rgb() = cyan.rgb();
				}

				//if (item[slot_function::ITEM_DEPOSIT]->for_categorized_items_only)
				//	label_color.rgb() = pink.rgb();
				//else
				//	label_color.rgb() = cyan.rgb();
			}

			if (bottom_number_val > -1.f) {
				std::wstring label_wstr;

				if (printing_charge_count) {
					//label_wstr = L'x';
					label_color.rgb() = white.rgb();
					label_wstr += to_wstring(bottom_number_val);
				}
				else
					label_wstr = to_wstring(bottom_number_val, 2);

				if (trim_zero && label_wstr[0] == L'0') {
					label_wstr.erase(label_wstr.begin());
				}

				// else label_wstr = L'{' + label_wstr + L'}';

				auto bottom_number = augs::gui::text::format(label_wstr, augs::gui::text::style(assets::font_id::GUI_FONT, label_color));

				charges_caption.set_text(bottom_number);
				charges_caption.bottom_right(get_rect_absolute());
				charges_caption.draw(in);
			}
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
			if (is_container_open)
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
	rect::perform_logic_step(gr);

	if (is_inventory_root()) {
		enable_drawing_of_children = true;
		disable_hovering = true;
		return;
	}

	enable_drawing_of_children = is_container_open && !is_being_wholely_dragged_or_pending_finish(gr);
	disable_hovering = is_being_wholely_dragged_or_pending_finish(gr);

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

void item_button::consume_gui_event(event_info info) {
	if (is_inventory_root())
		return;

	auto& gui = (game_gui_world&)info.owner;

	detector.update_appearance(info);
	auto parent_slot = item->get<components::item>().current_slot;

	if (info == rect::gui_event::ldrag) {
		if (!started_drag) {
			started_drag = true;

			gui.dragged_charges = item->get<components::item>().charges;

			if (parent_slot->always_allow_exactly_one_item)
				if (get_meta(parent_slot).get_rect_absolute().hover(info.owner.state.mouse.pos)) {
					get_meta(parent_slot).houted_after_drag_started = false;
				}
		}
	}

	if (info == rect::gui_event::wheel) {
		LOG("%x", info.owner.state.mouse.scroll);
	}

	if (info == rect::gui_event::rclick) {
		is_container_open = !is_container_open;
	}

	if (info == rect::gui_event::lfinisheddrag) {
		started_drag = false;

		auto& parent_cosmos = item->get_owner_world();
		auto& drag_result = gui.prepare_drag_and_drop_result();

		if (drag_result.possible_target_hovered && drag_result.will_drop_be_successful()) {
			parent_cosmos.post_message(drag_result.intent);
		}
		else if (!drag_result.possible_target_hovered) {
			vec2i griddified = griddify(info.owner.current_drag_amount);

			if (parent_slot->always_allow_exactly_one_item) {
				get_meta(parent_slot).user_drag_offset += griddified;
				get_meta(parent_slot).houted_after_drag_started = true;
				get_meta(parent_slot).perform_logic_step(info.owner);
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

	if (!is_being_wholely_dragged_or_pending_finish(in.owner)) {
		draw_complete_with_children(in);
	}
}

item_button& get_meta(entity_id id) {
	return get_owning_transfer_capability(id)->get<components::gui_element>().item_metadata[id];
}
