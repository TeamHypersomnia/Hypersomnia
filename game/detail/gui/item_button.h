#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory_slot_id.h"
#include "augs/gui/element_handle.h"
#include "game/entity_handle.h"

class item_button : public augs::gui::rect{
	void draw_dragged_ghost_inside(augs::gui::draw_info in);
	void draw_complete_with_children(augs::gui::draw_info in);

	rects::ltrb<float> iterate_children_attachments(bool draw = false, std::vector<vertex_triangle>* target = nullptr, augs::rgba col = augs::white);
	rects::ltrb<float> with_attachments_bbox;
public:
	item_button(rects::xywh<float> rc = rects::xywh<float>());

	bool is_being_wholely_dragged_or_pending_finish(augs::gui::rect_world& gr);

	augs::gui::text_drawer charges_caption;
	

	augs::gui::appearance_detector detector;

	bool is_container_open = false;

	bool started_drag = false;

	entity_id item;

	vec2i drag_offset_in_item_deposit;
};

template <bool is_const>
maybe_const_ref_t<is_const, item_button> get_meta(basic_entity_handle<is_const> id) {
	return id.get_owning_transfer_capability().get<components::gui_element>().item_metadata[id];
}

namespace augs {
	namespace gui {
		template<bool is_const>
		class element_handle_userdata<is_const, item_button> {
		public:
			basic_entity_handle<is_const> gui_element_entity;
		};

		template<bool is_const, class... all_elements>
		class basic_element_handle<item_button, is_const, all_elements...>
			: public basic_element_handle_base<item_button, is_const, all_elements...> {

			template<class = std::enable_if_t<!is_const>>
			void logic() const;

			template<class = std::enable_if_t<!is_const>>
			void consume_gui_event(event_info) const;

			bool is_inventory_root() const;

			void draw(draw_info) const;

			void draw_grid_border_ghost(draw_info in) const;
			void draw_complete_dragged_ghost(draw_info) const;

			void draw_proc(draw_info, bool draw_inside,
				bool draw_border,
				bool draw_connector,
				bool decrease_alpha,
				bool decrease_border_alpha = false,
				bool draw_container_opened_mark = false,
				bool draw_charges = true) const;
		};
	}
}
