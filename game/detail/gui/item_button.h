#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory_slot_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/resources/manager.h"

#include "augs/padding_byte.h"
#include "game/resources/manager.h"

#include "dispatcher_context.h"

struct item_button : game_gui_rect_node {
	typedef augs::gui::draw_info draw_info;
	typedef game_gui_rect_node base;

	typedef item_button_location location;

	template <class C, class gui_element_id, class L>
	static void for_each_child(C context, const gui_element_id& this_id, L generic_call) {
		const auto container = context.get_step().get_cosmos()[this_id.get_location().item_id];

		auto* maybe_container = container.find<components::container>();

		if (maybe_container) {
			for (auto& s : maybe_container->slots) {
				generic_call(make_location_and_pointer(&s.second.button, slot_button::location{ s.first }));
			}
		}
	}

	struct drawing_flags {
		bool draw_inside = false;
		bool draw_border = false;
		bool draw_connector = false;
		bool decrease_alpha = false;
		bool decrease_border_alpha = false;
		bool draw_container_opened_mark = false;
		bool draw_charges = true;
	};

	augs::gui::appearance_detector detector;
	rects::ltrb<float> with_attachments_bbox;

	bool is_container_open = false;
	bool started_drag = false;
	padding_byte pad[2];

	vec2i drag_offset_in_item_deposit;

	item_button(rects::xywh<float> rc = rects::xywh<float>());

	void draw_dragged_ghost_inside(augs::gui::draw_info in);
	void draw_complete_with_children(augs::gui::draw_info in);

	template <class C, class gui_element_id>
	rects::ltrb<float> iterate_children_attachments(
		const C& context, 
		const gui_element_id& this_id,
		const bool draw = false, 
		std::vector<vertex_triangle>* target = nullptr, 
		augs::rgba col = augs::white
	) {
		const auto& location = this_id.get<item_button_for_item_component_location>();
		const auto& item_handle = context.get_step().get_cosmos()[location.item_id];

		auto item_sprite = item_handle.get<components::sprite>();

		const auto& gui_def = resource_manager.find(item_sprite.tex)->gui_sprite_def;

		item_sprite.flip_horizontally = gui_def.flip_horizontally;
		item_sprite.flip_vertically = gui_def.flip_vertically;
		item_sprite.rotation_offset = gui_def.rotation_offset;

		item_sprite.color.a = border_col.a;

		components::sprite::drawing_input state(*target);
		state.screen_space_mode = true;

		auto expanded_size = rc.get_size() - with_attachments_bbox.get_size();

		state.renderable_transform.pos = get_absolute_xy() - with_attachments_bbox.get_position() + expanded_size / 2 + vec2(1, 1);

		rects::ltrb<float> button_bbox = item_sprite.get_aabb(components::transform(), true);

		if (!is_container_open) {
			for_each_descendant(item, [this, draw, &item_sprite, &state, &button_bbox](entity_id desc) {
				if (desc == item)
					return;

				auto parent_slot = desc->get<components::item>().current_slot;

				if (parent_slot.should_item_inside_keep_physical_body(item)) {
					auto attachment_sprite = desc->get<components::sprite>();

					attachment_sprite.flip_horizontally = item_sprite.flip_horizontally;
					attachment_sprite.flip_vertically = item_sprite.flip_vertically;
					attachment_sprite.rotation_offset = item_sprite.rotation_offset;

					attachment_sprite.color.a = item_sprite.color.a;
					shared::state_for_drawing_renderable attachment_state = state;
					auto offset = parent_slot.sum_attachment_offsets_of_parents(desc) - item->get<components::item>().current_slot.sum_attachment_offsets_of_parents(item);

					if (attachment_sprite.flip_horizontally) {
						offset.pos.x = -offset.pos.x;
						offset.flip_rotation();
					}

					if (attachment_sprite.flip_vertically) {
						offset.pos.y = -offset.pos.y;
						offset.flip_rotation();
					}

					offset += item_sprite.size / 2;
					offset += -attachment_sprite.size / 2;

					attachment_state.renderable_transform += offset;

					if (draw) {
						attachment_sprite.draw(attachment_state);
					}

					rects::ltrb<float> attachment_bbox = attachment_sprite.get_aabb(offset, true);
					button_bbox.contain(attachment_bbox);
				}
			});
		}

		if (draw) {
			item_sprite.draw(state);
		}

		return button_bbox;
	}

	static bool is_being_wholely_dragged_or_pending_finish(const const_dispatcher_context&, const location& this_id);

	static void consume_gui_event(const dispatcher_context&, const location& this_id, const gui_event);
	static void perform_logic_step(const dispatcher_context&, const location& this_id, const fixed_delta& delta);

	static bool is_inventory_root(const const_dispatcher_context&, const location& this_id);
	static void draw_triangles(const const_dispatcher_context&, const location& this_id, draw_info);
	static void draw_grid_border_ghost(const const_dispatcher_context&, draw_info in);
	static void draw_complete_dragged_ghost(const const_dispatcher_context&, draw_info);

	static void draw_proc(draw_info, const drawing_flags&);
};