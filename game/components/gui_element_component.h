#pragma once
#include "augs/gui/rect.h"
#include "game/entity_id.h"
#include <map>

#include "game/detail/inventory_slot_id.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"

#include "game/detail/gui/game_gui_root.h"
#include "game/detail/gui/immediate_hud.h"

class viewing_step;

namespace components {
	struct gui_element {
		vec2 gui_crosshair_position;
		
		aabb_highlighter world_hover_highlighter;
		
		int dragged_charges = 0;
		
		vec2i size;
		
		bool is_gui_look_enabled = false;
		bool preview_due_to_item_picking_request = false;
		bool draw_free_space_inside_container_icons = true;
		
		immediate_hud hud;
		
		special_drag_and_drop_target drop_item_icon = special_drag_and_drop_target(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red));
		
		std::map<inventory_slot_id, slot_button> slot_metadata;
		std::map<entity_id, item_button> item_metadata;
		
		std::map<inventory_slot_id, slot_button> removed_slot_metadata;
		std::map<entity_id, item_button> removed_item_metadata;
		
		rects::xywh<float> get_rectangle_for_slot_function(slot_function) const;
		vec2i get_initial_position_for_special_control(special_control) const;
		vec2 initial_inventory_root_position() const;
		void draw_complete_gui_for_camera_rendering_request(viewing_step&) const;
	};
}