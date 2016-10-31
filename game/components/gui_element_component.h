#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/gui/drag_and_drop_target_drop_item.h"
#include "game/detail/gui/location_and_pointer.h"
#include "game/enums/slot_function.h"
#include "augs/ensure.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "game/detail/gui/dispatcher_context.h"
//#include <map>
//
//#include "game/detail/inventory_slot_id.h"
//#include "augs/gui/appearance_detector.h"
//
//
//#include "game/detail/augs/gui/immediate_hud.h"
//

class viewing_step;
class logic_step;

namespace components {
	struct gui_element {
		game_gui_rect_world rect_world;
		vec2 gui_crosshair_position;
		int dragged_charges = 0;
		vec2i size;

		bool is_gui_look_enabled = false;
		bool preview_due_to_item_picking_request = false;
		bool draw_free_space_inside_container_icons = true;
		padding_byte pad;

		drag_and_drop_target_drop_item drop_item_icon;

		//augs::constant_size_associative_vector<inventory_slot_id, slot_button, GUI_ELEMENT_METADATA_COUNT> slot_metadata;
		//augs::constant_size_associative_vector<entity_id, item_button, GUI_ELEMENT_METADATA_COUNT> item_metadata;
		//
		//decltype(gui_element::slot_metadata) removed_slot_metadata;
		//decltype(gui_element::item_metadata) removed_item_metadata;
		
		gui_element();
		//
		//void consume_raw_input(augs::window::event::change&);
		//
		//drag_and_drop_result prepare_drag_and_drop_result() const;
		//

		rects::xywh<float> get_rectangle_for_slot_function(const slot_function) const;
		
		vec2i get_initial_position_for(const drag_and_drop_target_drop_item&) const;
		vec2 initial_inventory_root_position() const;
		
		void draw_cursor_and_tooltip(const viewing_dispatcher_context&) const;
		
		static entity_id get_hovered_world_entity(const cosmos& cosm, const vec2& world_cursor_position);
		static void draw_complete_gui_for_camera_rendering_request(const const_entity_handle& handle, viewing_step&);
	};
}