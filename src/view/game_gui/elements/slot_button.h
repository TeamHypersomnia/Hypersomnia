#pragma once
#include "augs/gui/gui_event.h"
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "view/game_gui/game_gui_context.h"

#include "augs/pad_bytes.h"

struct slot_button : game_gui_rect_node {
	using this_in_container = dereferenced_location<slot_button_in_container>;
	using const_this_in_container = const_dereferenced_location<slot_button_in_container>;
	using gui_entropy = game_gui_rect_node::gui_entropy;

	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	slot_button();

	static void respond_to_events(const game_gui_context, const this_in_container, const gui_entropy& entropies);
	static void rebuild_layouts(const game_gui_context, const this_in_container this_id);
	static void draw(const viewing_game_gui_context, const const_this_in_container);

	static void update_rc(const game_gui_context, const this_in_container);
};