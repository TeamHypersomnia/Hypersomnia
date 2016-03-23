#include "game_framework/systems/gui_system.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/item_component.h"
#include "graphics/renderer.h"
#include "gui/stroke.h"
#include "../inventory_utils.h"

#include "ensure.h"

using namespace augs;
using namespace gui;

bool gui_system::drag_and_drop_result::will_drop_be_successful() {
	return result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER;
}

gui_system::drag_and_drop_result gui_system::prepare_drag_and_drop_result() {
	drag_and_drop_result out;
	auto& tooltip_text = out.tooltip_text;

	if (gui.held_rect_is_dragged) {
		auto*& dragged_item = out.dragged_item;

		dragged_item = dynamic_cast<item_button*>(gui.rect_held_by_lmb);

		if (dragged_item && gui.rect_hovered) {
			slot_button* target_slot = dynamic_cast<slot_button*>(gui.rect_hovered);
			item_button* target_item = dynamic_cast<item_button*>(gui.rect_hovered);

			out.possible_target_hovered = true;

			messages::item_slot_transfer_request simulated_request;
			simulated_request.item = dragged_item->item;

			bool was_pointing_to_a_stack_target = false;
			bool no_slot_in_targeted_item = false;

			if (target_slot && target_slot->houted_after_drag_started)
				simulated_request.target_slot = target_slot->slot_id;
			else if (target_item && target_item != dragged_item) {
				if (target_item->item->find<components::container>())
					simulated_request.target_slot = target_item->item[detect_compatible_slot(dragged_item->item, target_item->item)];
				else if (can_merge_entities(target_item->item, dragged_item->item)) {
					simulated_request.target_slot = target_item->item->get<components::item>().current_slot;
					was_pointing_to_a_stack_target = true;
				}
				else
					no_slot_in_targeted_item = true;
			}
			else
				out.possible_target_hovered = false;

			if (out.possible_target_hovered) {
				if (no_slot_in_targeted_item) {
					out.result.result = item_transfer_result_type::NO_SLOT_AVAILABLE;
					out.result.transferred_charges = 0;
				}
				else
					out.result = query_transfer_result(simulated_request);
				
				out.intent = simulated_request;

				auto predicted_result = out.result.result;

				if (predicted_result == item_transfer_result_type::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result >= item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					if (predicted_result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					if (was_pointing_to_a_stack_target) {
						tooltip_text += L"Stack";
					}
					else {
						switch (simulated_request.target_slot.type) {
						case slot_function::ITEM_DEPOSIT: tooltip_text += L"Insert"; break;
						case slot_function::GUN_CHAMBER: tooltip_text += L"Place"; break;
						case slot_function::GUN_CHAMBER_MAGAZINE: tooltip_text += L"Place"; break;
						case slot_function::GUN_DETACHABLE_MAGAZINE: tooltip_text += L"Reload"; break;
						case slot_function::GUN_RAIL: tooltip_text += L"Install"; break;
						case slot_function::TORSO_ARMOR_SLOT: tooltip_text += L"Wear"; break;
						case slot_function::SHOULDER_SLOT: tooltip_text += L"Wear"; break;
						case slot_function::PRIMARY_HAND: tooltip_text += L"Wield"; break;
						case slot_function::SECONDARY_HAND: tooltip_text += L"Wield"; break;
						case slot_function::GUN_BARREL: tooltip_text += L"Install"; break;
						default: ensure(0); break;
						}
					}
				}
				else if (predicted_result < item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					switch (predicted_result) {
					case item_transfer_result_type::INSUFFICIENT_SPACE: tooltip_text = L"No space"; break;
					case item_transfer_result_type::INVALID_SLOT_OR_UNOWNED_ROOT: tooltip_text = L"Impossible"; break;
					case item_transfer_result_type::INCOMPATIBLE_CATEGORIES: tooltip_text = L"Incompatible item"; break;
					case item_transfer_result_type::NO_SLOT_AVAILABLE: tooltip_text = L"No slot available"; break;
					default: ensure(0); break;
					}
				}
			}
		}
	}

	return out;
}

void gui_system::draw_cursor_and_tooltip(messages::camera_render_request_message r) {
	auto& drag_result = prepare_drag_and_drop_result();

	shared::state_for_drawing_renderable state;
	state.setup_camera_state(r.state);
	state.screen_space_mode = true;

	auto& out = state.output->get_triangle_buffer();
	gui::rect::draw_info in(gui, out);

	if (drag_result.dragged_item) {
		drag_result.dragged_item->draw_complete_dragged_ghost(in);

		if (!drag_result.possible_target_hovered)
			drag_result.dragged_item->draw_grid_border_ghost(in);
	}

	components::sprite bg_sprite;
	bg_sprite.set(assets::BLANK, black);
	bg_sprite.color.a = 120;	

	auto gui_cursor = assets::GUI_CURSOR;
	auto gui_cursor_color = cyan;

	if (drag_result.possible_target_hovered) {
		if (drag_result.will_drop_be_successful()) {
			gui_cursor = assets::GUI_CURSOR_ADD;
			gui_cursor_color = green;
		}
		else if(drag_result.result.result != item_transfer_result_type::THE_SAME_SLOT) {
			gui_cursor = assets::GUI_CURSOR_ERROR;
			gui_cursor_color = red;
		}
	}

	components::sprite cursor_sprite;
	cursor_sprite.set(gui_cursor, gui_cursor_color);

	bool draw_tooltip = drag_result.possible_target_hovered;
	if (draw_tooltip) {
		vec2i left_top_corner = gui_crosshair_position;

		tooltip_drawer.set_text(text::format(drag_result.tooltip_text, text::style()));
		tooltip_drawer.pos = gui_crosshair_position + vec2i(cursor_sprite.size.x + 2, 0);

		state.renderable_transform.pos = left_top_corner;
		bg_sprite.size.set(tooltip_drawer.get_bbox());
		bg_sprite.size.y = std::max(int(cursor_sprite.size.y), tooltip_drawer.get_bbox().y);
		bg_sprite.size.x += cursor_sprite.size.x;
		bg_sprite.draw(state);

		gui::solid_stroke stroke;
		stroke.set_material(gui::material(assets::BLANK, slightly_visible_white));
		
		stroke.draw(out, rects::ltrb<float>(gui_crosshair_position, bg_sprite.size));

		tooltip_drawer.draw_stroke(out, black);
		tooltip_drawer.draw(out);
	}

	state.renderable_transform.pos = gui_crosshair_position;
	cursor_sprite.draw(state);
}