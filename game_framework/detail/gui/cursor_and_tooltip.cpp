#include "game_framework/systems/gui_system.h"
#include "game_framework/components/sprite_component.h"
#include "graphics/renderer.h"

#include "../inventory_utils.h"

using namespace augs;
using namespace gui;

void gui_system::draw_cursor_and_tooltip(messages::camera_render_request_message r) {
	auto gui_cursor = assets::GUI_CURSOR;
	auto gui_cursor_color = cyan;

	std::wstring tooltip_text = L"";

	if (gui.held_rect_is_dragged) {
		item_button* dragged_item = dynamic_cast<item_button*>(gui.rect_held_by_lmb);

		if (dragged_item && gui.rect_hovered) {
			slot_button* target_slot = dynamic_cast<slot_button*>(gui.rect_hovered);
			item_button* target_item = dynamic_cast<item_button*>(gui.rect_hovered);

			std::pair<item_transfer_result, slot_function> predicted_result;
			predicted_result.second = slot_function::INVALID;

			bool queried = true;

			if (target_slot)
				predicted_result = { query_transfer_result({ dragged_item->item, target_slot->slot_id }), target_slot->slot_id.type };
			else if (target_item && target_item != dragged_item)
				predicted_result = query_transfer_result(dragged_item->item, target_item->item);
			else
				queried = false;

			if (queried) {
				if (predicted_result.first == item_transfer_result::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result.first >= item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ADD;
					gui_cursor_color = green;

					if (predicted_result.first == item_transfer_result::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					switch (predicted_result.second) {
					case slot_function::ITEM_DEPOSIT: tooltip_text = L"Insert"; break;
					case slot_function::GUN_CHAMBER: tooltip_text = L"Place"; break;
					case slot_function::GUN_CHAMBER_MAGAZINE: tooltip_text = L"Place"; break;
					case slot_function::GUN_DETACHABLE_MAGAZINE: tooltip_text = L"Reload"; break;
					case slot_function::GUN_RAIL: tooltip_text = L"Install"; break;
					case slot_function::TORSO_ARMOR_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::SHOULDER_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::PRIMARY_HAND: tooltip_text = L"Pull out"; break;
					case slot_function::SECONDARY_HAND: tooltip_text = L"Pull out"; break;
					case slot_function::GUN_BARREL: tooltip_text = L"Install"; break;
					default: assert(0); break;
					}
				}
				else if (predicted_result.first < item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ERROR;
					gui_cursor_color = red;

					switch (predicted_result.first) {
					case item_transfer_result::INSUFFICIENT_SPACE: tooltip_text = L"No space"; break;
					case item_transfer_result::INVALID_SLOT_OR_UNOWNED_ROOT: tooltip_text = L"Impossible"; break;
					case item_transfer_result::INCOMPATIBLE_CATEGORIES: tooltip_text = L"Incompatible item"; break;
					case item_transfer_result::NO_SLOT_AVAILABLE: tooltip_text = L"No slot available"; break;
					default: assert(0); break;
					}
				}
			}
		}
	}

	components::sprite cursor_sprite;
	cursor_sprite.set(gui_cursor);
	cursor_sprite.color = gui_cursor_color;

	shared::state_for_drawing_renderable state;
	state.setup_camera_state(r.state);
	state.screen_space_mode = true;
	state.renderable_transform.pos = gui_crosshair_position;

	cursor_sprite.draw(state);

	if (tooltip_text.size() > 0) {
		tooltip_drawer.set_text(text::format(tooltip_text, text::style()));

		tooltip_drawer.above_left_to_right(gui_crosshair_position);

		auto& out = state.output->get_triangle_buffer();

		tooltip_drawer.draw_stroke(out, black);
		tooltip_drawer.draw(out);
	}
}