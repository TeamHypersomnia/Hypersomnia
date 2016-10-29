#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/gui/special_drag_and_drop_target.h"
#include "game/detail/gui/location_and_pointer.h"
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
		class gui_tree_entry {
			const augs::gui::rect_node_data& node_data;
			gui_element_location parent;
			vec2 absolute_position;
		public:
			gui_tree_entry(const augs::gui::rect_node_data& node_data) : node_data(node_data) {}

			void set_parent(const gui_element_location& id) {
				parent = id;
			}

			void set_absolute_clipping_rect(const rects::ltrb<float>&) {

			}

			rects::ltrb<float> set_absolute_clipped_rect(const rects::ltrb<float>&) {

			}

			void set_absolute_pos(const vec2& v) {
				absolute_position = v;
			}

			gui_element_location get_parent() const {
				return parent;
			}

			rects::ltrb<float> get_absolute_rect() const {
				return rects::xywh<float>(absolute_position.x, absolute_position.y, node_data.rc.w(), node_data.rc.h());
			}

			rects::ltrb<float> get_absolute_clipping_rect() const {
				return rects::ltrb<float>(0.f, 0.f, std::numeric_limits<int>::max() / 2.f, std::numeric_limits<int>::max() / 2.f);
			}

			rects::ltrb<float> get_absolute_clipped_rect() const {
				return node_data.rc;
			}

			vec2 get_absolute_pos() const {
				return absolute_position;
			}
		};

		typedef std::unordered_map<gui_element_location, gui_tree_entry> gui_element_tree;

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
		//void consume_raw_input(augs::window::event::change&);
		//
		//drag_and_drop_result prepare_drag_and_drop_result() const;
		//

		rects::xywh<float> get_rectangle_for_slot_function(const slot_function) const;
		vec2i get_initial_position_for_special_control(const special_control) const;
		vec2 initial_inventory_root_position() const;
		
		void draw_cursor_and_tooltip(const const_dispatcher_context&) const;
		
		static entity_id get_hovered_world_entity(const cosmos& cosm, const vec2& world_cursor_position);
		static void draw_complete_gui_for_camera_rendering_request(const const_entity_handle& handle, viewing_step&);
	};
}