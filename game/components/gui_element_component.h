#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/gui/special_drag_and_drop_target.h"
#include "game/enums/slot_function.h"
#include "augs/ensure.h"

#include "augs/misc/constant_size_vector.h"
//#include <map>
//
//#include "game/detail/inventory_slot_id.h"
//#include "augs/gui/appearance_detector.h"
//
//#include "game/detail/augs/gui/item_button.h"
//#include "game/detail/augs/gui/slot_button.h"
//
//#include "game/detail/augs/gui/game_gui_root.h"
//#include "game/detail/augs/gui/immediate_hud.h"
//
//#include "augs/gui/element_world.h"

class viewing_step;
class fixed_step;

namespace components {
	struct gui_element {
		class dispatcher_context {
		public:
			fixed_step& step;
			gui_element& elem;

			gui_element& get_gui_element_component() {
				return elem;
			}

			augs::gui::rect_world& get_rect_world() {
				return elem.rect_world;
			}

			bool alive(const augs::gui::gui_element_id& id) const {
				if (id == elem.drop_item_icon.this_id) {
					return true;
				}
			}

			template<class L>
			decltype(auto) operator()(const augs::gui::gui_element_id& id, L polymorphic_call) {
				if (id == elem.drop_item_icon.this_id) {
					return polymorphic_call(drop_item_icon);
				}
				else {
					
				}

				ensure(false);

				augs::gui::rect_leaf def;
				return polymorphic_call(def);
			}
		};

		unsigned element_guid_counter = 0;

		augs::gui::rect_world rect_world;
		augs::gui::gui_element_id assign_guid();
		//augs::gui::element_world<slot_button, item_button, special_drag_and_drop_target> elements;
		//
		//vec2 gui_crosshair_position;
		//
		//
		//int dragged_charges = 0;
		//
		vec2i size;
		//
		//bool is_gui_look_enabled = false;
		//bool preview_due_to_item_picking_request = false;
		//bool draw_free_space_inside_container_icons = true;
		//
		//
		special_drag_and_drop_target drop_item_icon;
		//
		//std::map<inventory_slot_id, slot_button> slot_metadata;
		//std::map<entity_id, item_button> item_metadata;
		//
		//std::map<inventory_slot_id, slot_button> removed_slot_metadata;
		//std::map<entity_id, item_button> removed_item_metadata;
		//
		gui_element();
		//
		//void consume_raw_input(augs::window::event::state&);
		//void draw_cursor_and_tooltip(viewing_step&) const;
		//
		//entity_id get_hovered_world_entity(vec2 camera_pos);
		//drag_and_drop_result prepare_drag_and_drop_result() const;
		//
		rects::xywh<float> get_rectangle_for_slot_function(slot_function) const;
		vec2i get_initial_position_for_special_control(special_control) const;
		vec2 initial_inventory_root_position() const;
		void draw_complete_gui_for_camera_rendering_request(viewing_step&) const;
	};
}