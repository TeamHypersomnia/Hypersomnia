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

		template <bool is_const>
		class basic_dispatcher_context {

		public:
			typedef std::conditional_t<is_const, viewing_step, fixed_step>& step_ref;
			typedef maybe_const_ref_t<is_const, gui_element> gui_element_ref;
			typedef maybe_const_ref_t<is_const, game_gui_rect_world> game_gui_rect_world_ref;

			basic_dispatcher_context(step_ref step, basic_entity_handle<is_const> handle, gui_element_ref elem, gui_element_tree& tree) :
				step(step), 
				handle(handle),
				//composite_for_iteration(parent),
				elem(elem),
				tree(tree)
				{}

			step_ref step;
			basic_entity_handle<is_const> handle;
			gui_element_ref elem;
			gui_element_tree& tree;

			basic_entity_handle<is_const> get_gui_element_entity() const {
				return handle;
			}
			//parent_of_inventory_controls_rect& composite_for_iteration;

			step_ref get_step() const {
				return step;
			}

			gui_element_ref get_gui_element_component() const {
				return elem;
			}

			game_gui_rect_world_ref get_rect_world() const {
				return elem.rect_world;
			}

			gui_tree_entry& get_tree_entry(const gui_element_location& id) {
				if (gui_tree.find(id) == gui_tree.end()) {
					gui_tree.emplace(id, gui_tree_entry(operator()(id, [](const auto& resolved_ref) { 
						return static_cast<const augs::gui::rect_node_data&>(resolved_ref); 
					})));
				}

				return gui_tree.at(id);
			}

			const gui_tree_entry& get_tree_entry(const gui_element_location& id) const {
				return gui_tree.at(id);
			}

			bool alive(const gui_element_location& id) const {
				return id.is_set() && id.call([this](const auto& resolved) {
					return resolved.alive(*this);
				});
			}
			
			bool dead(const gui_element_location& id) const {
				return !alive(id);
			}

			operator basic_dispatcher_context<true>() const {
				return{ step, handle, elem, tree };
			}

			template<class L>
			decltype(auto) operator()(const gui_element_location& id, L generic_call) const {
				return id.call([&](const auto& resolved_location) {
					return resolved_location.get_object_at_location_and_call(*this, generic_call);
				});
			}

			template<class Casted>
			struct pointer_caster {
				template <class Candidate>
				maybe_const_ptr_t<is_const, Casted> operator()(maybe_const_ref_t<is_const, Candidate> object) {
					if (std::is_same<Casted, Candidate>::value || std::is_base_of<Casted, Candidate>::value) {
						return reinterpret_cast<Casted*>(&object);
					}

					return nullptr;
				}
			};

			template<class T>
			maybe_const_ptr_t<is_const, T> get_pointer(const gui_element_location& id) const {
				if (dead(id)) {
					return nullptr;
					//return static_cast<maybe_const_ptr<is_const, T>>(nullptr);
				}

				return id.call([&](const auto& resolved_location) const {
					return resolved_location.get_object_at_location_and_call(*this, pointer_caster<T>());
				});
			}

			template<class T>
			maybe_const_ptr_t<is_const, T> get_alive_location_pointer(const gui_element_location& id) const {
				static_assert(tuple_contains_type<T, typename decltype(id)::types_tuple>::value, "Invalid location type!");

				if (dead(id)) {
					return nullptr;
				}

				return &id.get<T>();
			}
		};

		typedef basic_dispatcher_context<false> dispatcher_context;
		typedef basic_dispatcher_context<true> const_dispatcher_context;

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
		//entity_id get_hovered_world_entity(vec2 camera_pos);
		//drag_and_drop_result prepare_drag_and_drop_result() const;
		//

		rects::xywh<float> get_rectangle_for_slot_function(const slot_function) const;
		vec2i get_initial_position_for_special_control(const special_control) const;
		vec2 initial_inventory_root_position() const;
		
		void draw_cursor_and_tooltip(const const_dispatcher_context&) const;
		static void draw_complete_gui_for_camera_rendering_request(const const_entity_handle& handle, viewing_step&);
	};
}