#pragma once
#include "augs/gui/gui_event.h"
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory_slot_id.h"
#include "game/detail/gui/game_gui_context.h"

#include "augs/padding_byte.h"

struct slot_button : game_gui_rect_node {
	typedef dereferenced_location<slot_button_in_container> this_in_container;
	typedef const_dereferenced_location<slot_button_in_container> const_this_in_container;

	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	slot_button();

	static void perform_logic_step(const logic_gui_context&, const this_in_container&);
	
	static void draw(const viewing_gui_context&, const const_this_in_container&, augs::gui::draw_info);
	static void consume_gui_event(logic_gui_context&, const this_in_container&, const augs::gui::event_info info);
};

slot_button& get_meta(inventory_slot_id);