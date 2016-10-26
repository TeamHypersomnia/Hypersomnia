#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/gui/special_drag_and_drop_target.h"
#include "game/enums/slot_function.h"
#include "augs/ensure.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "game/detail/gui/gui_element_location.h"
//#include <map>
//
//#include "game/detail/inventory_slot_id.h"
//#include "augs/gui/appearance_detector.h"
//
#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"
//
//#include "game/detail/augs/gui/immediate_hud.h"
//

class viewing_step;
class fixed_step;

namespace components {
	struct gui_element {
		//struct parent_of_inventory_controls_rect : game_gui_rect_leaf {
		//	parent_of_inventory_controls_rect(gui_element& element) : rect_leaf(make_guid(PARENT_OF_ALL_CONTROLS)), elem(element) {}
		//
		//	gui_element& elem;
		//
		//	template<class C>
		//	void for_each_child(C context) {
		//
		//	}
		//};

		class dispatcher_context {
		public:
			dispatcher_context(fixed_step& step, gui_element& elem) :
				step(step), 
				//composite_for_iteration(parent),
				elem(elem) {}

			fixed_step& step;
			gui_element& elem;
			//parent_of_inventory_controls_rect& composite_for_iteration;

			fixed_step& get_step() {
				return step;
			}

			gui_element& get_gui_element_component() {
				return elem;
			}

			augs::gui::rect_world<gui_element_location>& get_rect_world() {
				return elem.rect_world;
			}

			//bool alive(const gui_element_location& id) const {
			//	if (id == elem.drop_item_icon.this_id) {
			//		return true;
			//	}
			//	else if (id == composite_for_iteration.this_id) {
			//		return true;
			//	}
			//
			//	return false;
			//}

			template<class L>
			decltype(auto) operator()(const gui_element_location& id, L polymorphic_call) {
				if (id == elem.drop_item_icon.this_id) {
					return polymorphic_call(drop_item_icon);
				}
				else if(id == composite_for_iteration.this_id) {
					return polymorphic_call(composite_for_iteration);
				}

				ensure(false);

				augs::gui::rect_leaf def;
				return polymorphic_call(def);
			}
		};

		game_gui_rect_world rect_world;
		vec2 gui_crosshair_position;
		int dragged_charges = 0;
		vec2i size;

		bool is_gui_look_enabled = false;
		bool preview_due_to_item_picking_request = false;
		bool draw_free_space_inside_container_icons = true;
		padding_byte pad;

		special_drag_and_drop_target drop_item_icon;

		//augs::constant_size_associative_vector<inventory_slot_id, slot_button, GUI_ELEMENT_METADATA_COUNT> slot_metadata;
		//augs::constant_size_associative_vector<entity_id, item_button, GUI_ELEMENT_METADATA_COUNT> item_metadata;
		//
		//decltype(gui_element::slot_metadata) removed_slot_metadata;
		//decltype(gui_element::item_metadata) removed_item_metadata;
		
		gui_element();
		//
		//void consume_raw_input(augs::window::event::state&);
		//void draw_cursor_and_tooltip(viewing_step&) const;
		//
		//entity_id get_hovered_world_entity(vec2 camera_pos);
		//drag_and_drop_result prepare_drag_and_drop_result() const;
		//

		void advance(fixed_step&);

		rects::xywh<float> get_rectangle_for_slot_function(const slot_function) const;
		vec2i get_initial_position_for_special_control(const special_control) const;
		vec2 initial_inventory_root_position() const;
		void draw_complete_gui_for_camera_rendering_request(viewing_step&) const;

	private:
		enum reserved_guid : unsigned {
			PARENT_OF_ALL_CONTROLS = 0
		};
	};
}