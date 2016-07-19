#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/element_handle.h"
#include "augs/gui/appearance_detector.h"
#include "special_controls.h"

struct special_drag_and_drop_target {
	template<class all_elements...>
	special_drag_and_drop_target(augs::gui::rect_handle rect, augs::gui::element_world<all_elements...>& world, augs::gui::material new_mat) {
		auto& self = rect.get();
		mat = new_mat;

		self.mat = mat;
		self.clip = false;
		self.enable_drawing = false;
	}

	special_control type;

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;
};

namespace augs {
	namespace gui {
		template<bool is_const, class... all_elements>
		class basic_element_handle<special_drag_and_drop_target, is_const, all_elements...>
			: public basic_element_handle_base<special_drag_and_drop_target, is_const, all_elements...> {

			basic_entity_handle<is_const> owner_entity;

			basic_element_handle(owner_reference owner, id_type id, basic_entity_handle<is_const> owner_entity) : basic_handle(owner, id), 
				owner_entity(owner_entity) {
			}

			void draw(draw_info info) const {
				auto mat_coloured = mat;

				if (detector.is_hovered)
					mat_coloured.color.a = 255;
				else
					mat_coloured.color.a = 120;

				get_rect().draw_centered_texture(info, mat_coloured);
			}

			template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
			void event(event_info) const {
				get().detector.update_appearance(info);
			}

			template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
			void logic() const {
				auto& gui = get_rect_world();
				auto& entity = userdata.owner_entity;

				auto dragged_item = owner[gui.rect_held_by_lmb];

				enable_drawing = dragged_item.alive() && gui.held_rect_is_dragged;
				rc.set_position(entity.get<components::gui_element>().get_initial_position_for_special_control(type) - vec2(20, 20));
				rc.set_size((*mat.tex).get_size() + vec2(40, 40));
			}
		};
	}
}
