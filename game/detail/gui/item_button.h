#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory_slot_id.h"
#include "augs/gui/element_handle.h"
#include "game/entity_handle.h"
#include "game/resources/manager.h"

struct item_button {
	template<class all_elements...>
	item_button(augs::gui::rect_handle rect, augs::gui::element_world<all_elements...>& world) {
		auto& self = rect.get();
		self.clip = false;
		self.scrollable = false;
		self.focusable = false;
	}

	rects::ltrb<float> with_attachments_bbox;

	augs::gui::text_drawer charges_caption;
	
	augs::gui::appearance_detector detector;

	bool is_container_open = false;

	bool started_drag = false;

	entity_id item;

	vec2i drag_offset_in_item_deposit;
};

namespace augs {
	namespace gui {
		template<bool is_const, class... all_elements>
		class basic_element_handle<item_button, is_const, all_elements...>
			: public basic_element_handle_base<item_button, is_const, all_elements...> {

			basic_entity_handle<is_const> gui_element_entity;

			basic_element_handle(owner_reference owner, id_type id, basic_entity_handle<is_const> gui_element_entity) : basic_handle(owner, id),
				gui_element_entity(gui_element_entity) {
			}

			template <bool is_const>
			basic_element_handle<item_button, is_const, all_elements...> get_corresponding_element(basic_entity_handle<is_const> id) {
				return owner[id.get_owning_transfer_capability().get<components::gui_element>().item_metadata[id], gui_element_entity];
			}

			template<class = std::enable_if_t<!is_const>>
			void logic() const {
				auto& r = get_rect();
				auto& self = get();
				auto& cosmos = gui_element_entity.get_cosmos();

				if (is_inventory_root()) {
					r.enable_drawing_of_children = true;
					r.disable_hovering = true;
					return;
				}

				r.enable_drawing_of_children = self.is_container_open && !is_being_wholely_dragged_or_pending_finish(gr);
				r.disable_hovering = is_being_wholely_dragged_or_pending_finish(gr);

				vec2i parent_position;

				auto item = cosmos[self.item];

				auto* sprite = item.find<components::sprite>();

				if (sprite) {
					with_attachments_bbox = iterate_children_attachments();
					vec2i rounded_size = with_attachments_bbox.get_size();
					rounded_size += 22;
					rounded_size += resource_manager.find(sprite->tex)->gui_sprite_def.gui_bbox_expander;
					rounded_size /= 11;
					rounded_size *= 11;
					//rounded_size.x = std::max(rounded_size.x, 33);
					//rounded_size.y = std::max(rounded_size.y, 33);
					r.rc.set_size(rounded_size);
				}

				auto parent_slot = cosmos[item.get<components::item>().current_slot];

				if (parent_slot->always_allow_exactly_one_item) {
					r.rc.set_position(get_corresponding_element(parent_slot).get_rect().rc.get_position());
				}
				else {
					r.rc.set_position(drag_offset_in_item_deposit);
				}
			}

			template<class = std::enable_if_t<!is_const>>
			void consume_gui_event(event_info info) const {
				if (is_inventory_root())
					return;

				auto& gui = gui_element_entity.get<components::gui_element>();
				auto& self = get();
				auto& cosmos = gui_element_entity.get_cosmos();

				self.detector.update_appearance(info);
				
				auto item = cosmos[self.item];
				auto parent_slot = cosmos[item.get<components::item>().current_slot];

				if (info == gui_event::ldrag) {
					if (!self.started_drag) {
						self.started_drag = true;

						gui.dragged_charges = item.get<components::item>().charges;

						if (parent_slot->always_allow_exactly_one_item)
							if (get_corresponding_element(parent_slot).get_rect().get_rect_absolute().hover(info.owner.state.mouse.pos)) {
								get_corresponding_element(parent_slot).get_rect().houted_after_drag_started = false;
							}
					}
				}

				if (info == gui_event::wheel) {
					LOG("%x", info.owner.state.mouse.scroll);
				}

				if (info == gui_event::rclick) {
					self.is_container_open = !self.is_container_open;
				}

				if (info == gui_event::lfinisheddrag) {
					self.started_drag = false;

					auto& drag_result = gui.prepare_drag_and_drop_result();

					if (drag_result.possible_target_hovered && drag_result.will_drop_be_successful()) {
						step.messages.post(drag_result.intent);
					}
					else if (!drag_result.possible_target_hovered) {
						vec2i griddified = griddify(info.owner.current_drag_amount);

						if (parent_slot->always_allow_exactly_one_item) {
							get_meta(parent_slot).user_drag_offset += griddified;
							get_meta(parent_slot).houted_after_drag_started = true;
							get_meta(parent_slot).perform_logic_step(info.owner);
						}
						else {
							drag_offset_in_item_deposit += griddified;
						}
					}
				}

				// if(being_dragged && inf == gui_event::lup)
			}

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

			void draw_dragged_ghost_inside(augs::gui::draw_info in) const;
			void draw_complete_with_children(augs::gui::draw_info in) const;

			rects::ltrb<float> iterate_children_attachments(bool draw = false, std::vector<vertex_triangle>* target = nullptr, augs::rgba col = augs::white) const {
				auto item_sprite = item.get<components::sprite>();

				const auto& gui_def = resource_manager.find(item_sprite.tex)->gui_sprite_def;

				item_sprite.flip_horizontally = gui_def.flip_horizontally;
				item_sprite.flip_vertically = gui_def.flip_vertically;
				item_sprite.rotation_offset = gui_def.rotation_offset;

				item_sprite.color.a = border_col.a;

				shared::state_for_drawing_renderable state;
				state.screen_space_mode = true;
				state.overridden_target_buffer = target;

				auto expanded_size = rc.get_size() - with_attachments_bbox.get_size();

				state.renderable_transform.pos = get_absolute_xy() - with_attachments_bbox.get_position() + expanded_size / 2 + vec2(1, 1);

				rects::ltrb<float> button_bbox = item_sprite.get_aabb(components::transform(), true);

				if (!is_container_open) {
					for_each_descendant(item, [this, draw, &item_sprite, &state, &button_bbox](entity_id desc) {
						if (desc == item)
							return;

						auto parent_slot = desc.get<components::item>().current_slot;

						if (parent_slot.should_item_inside_keep_physical_body(item)) {
							auto attachment_sprite = desc.get<components::sprite>();

							attachment_sprite.flip_horizontally = item_sprite.flip_horizontally;
							attachment_sprite.flip_vertically = item_sprite.flip_vertically;
							attachment_sprite.rotation_offset = item_sprite.rotation_offset;

							attachment_sprite.color.a = item_sprite.color.a;
							shared::state_for_drawing_renderable attachment_state = state;
							auto offset = parent_slot.sum_attachment_offsets_of_parents(desc) - item.get<components::item>().current_slot.sum_attachment_offsets_of_parents(item);

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

							if (draw)
								attachment_sprite.draw(attachment_state);

							rects::ltrb<float> attachment_bbox = attachment_sprite.get_aabb(offset, true);
							button_bbox.contain(attachment_bbox);
						}
					});
				}

				if (draw)
					item_sprite.draw(state);

				return button_bbox;
			}

			bool is_being_wholely_dragged_or_pending_finish() const {
				auto& gui_element = gui_element_entity.get<components::gui_element>();

				if (get_rect().is_being_dragged(get_rect_world())) {
					bool is_drag_partial = (gui_element.dragged_charges < item.get<components::item>().charges;
					return !is_drag_partial;
				}

				return false;
			}
		};
	}
}
