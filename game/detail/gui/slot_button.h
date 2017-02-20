#pragma once
#include "augs/gui/gui_event.h"
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/gui/game_gui_context.h"

#include "augs/padding_byte.h"

struct slot_button : game_gui_rect_node {
	typedef dereferenced_location<slot_button_in_container> this_in_container;
	typedef const_dereferenced_location<slot_button_in_container> const_this_in_container;
	typedef game_gui_rect_node::gui_entropy gui_entropy;

	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	slot_button();

	static void respond_to_events(const game_gui_context, const this_in_container, const gui_entropy& entropies);
	static void rebuild_layouts(const game_gui_context, const this_in_container this_id);
	static void draw(const viewing_game_gui_context, const const_this_in_container, augs::gui::draw_info);

	static void update_rc(const game_gui_context, const this_in_container);
};

slot_button& get_meta(inventory_slot_id);