#include "hotbar_button.h"
#include "augs/gui/button_corners.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/gui/item_button.h"
#include "game/components/item_component.h"

void hotbar_button::associate_entity(const const_entity_handle h) {
	last_associated_entity = h.get_id();
}

const_entity_handle hotbar_button::get_associated_entity(const cosmos& cosm) const {
	return cosm[last_associated_entity];
}

entity_handle hotbar_button::get_associated_entity(cosmos& cosm) const {
	return cosm[last_associated_entity];
}

vec2i hotbar_button::get_bbox(const cosmos& cosm) const {
	const auto ent = get_associated_entity(cosm);

	if (ent.dead()) {
		return { 55, 55 };
	}

	return item_button::calculate_button_layout(ent, true).aabb.get_size();
}

void hotbar_button::draw(const viewing_gui_context& context, const const_this_in_item& this_id, draw_info in) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	const auto& rect_world = context.get_rect_world();
	const auto& this_tree_entry = context.get_tree_entry(this_id);

	const auto& detector = this_id->detector;

	const auto colorize = cyan;

	rgba inside_col = white;
	rgba border_col = white;

	inside_col.a = 20;
	border_col.a = 190;

	if (detector.is_hovered) {
		inside_col.a = 30;
		border_col.a = 220;
	}

	const bool pushed = detector.current_appearance == augs::gui::appearance_detector::appearance::pushed;

	if (pushed) {
		inside_col.a = 60;
		border_col.a = 255;
	}

	inside_col *= colorize;
	border_col *= colorize;

	const auto inside_mat = augs::gui::material(assets::texture_id::HOTBAR_BUTTON_INSIDE, inside_col);

	const bool flip = true;

	button_corners_info corners;
	corners.lt_texture = assets::texture_id::HOTBAR_BUTTON_LT;
	corners.flip_horizontally = flip;
	button_corners_info border_corners;
	border_corners.lt_texture = assets::texture_id::HOTBAR_BUTTON_LT_BORDER;
	border_corners.flip_horizontally = flip;

	const auto internal_rc = corners.cornered_rc_to_internal_rc(this_id->rc);

	augs::gui::draw_clipped_rect(inside_mat, internal_rc, {}, in.v);

	{
		corners.for_each_button_corner(internal_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
			if (type != button_corner_type::LB_COMPLEMENT) {
				augs::gui::draw_clipped_rect(augs::gui::material(id, inside_col), drawn_rc, {}, in.v, flip);
			}
		});

		border_corners.for_each_button_corner(internal_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
			if (type != button_corner_type::LB_COMPLEMENT) {
				augs::gui::draw_clipped_rect(augs::gui::material(id, border_col), drawn_rc, {}, in.v, flip);
			}
		});
		
		if (this_id->detector.is_hovered) {
			auto hover_effect_rc = internal_rc;
		
			if (pushed) {
				const auto distance = 4.f;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				border_corners.for_each_button_corner(hover_effect_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
					if (type != button_corner_type::LB_COMPLEMENT) {
						augs::gui::draw_clipped_rect(augs::gui::material(id, colorize), drawn_rc, {}, in.v, true);
					}
				});
			}
			else {
				const auto max_duration = this_id->hover_highlight_duration_ms;
				const auto max_distance = this_id->hover_highlight_maximum_distance;
		
				const auto distance = (1.f - std::min(max_duration, this_id->elapsed_hover_time_ms) / max_duration) * max_distance;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				border_corners.for_each_button_corner(hover_effect_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
					if (type != button_corner_type::LB_COMPLEMENT && is_button_corner(type)) {
						augs::gui::draw_clipped_rect(augs::gui::material(id, colorize), drawn_rc, {}, in.v, true);
					}
				});
			}
		}
	}

	const auto& cosmos = context.get_step().get_cosmos();
	const auto associated_entity = this_id->get_associated_entity(cosmos);

	if (associated_entity.alive() && associated_entity.get_owning_transfer_capability() == context.get_gui_element_entity()) {
		ensure(associated_entity.has<components::item>());

		item_button_in_item location;
		location.item_id = this_id->last_associated_entity;

		const auto dereferenced = context.dereference_location(location);
		ensure(dereferenced != nullptr);

		item_button::drawing_settings f;
		f.draw_background = false;
		f.draw_item = true;
		f.draw_border = false;
		f.draw_connector = false;
		f.decrease_alpha = false;
		f.decrease_border_alpha = false;
		f.draw_container_opened_mark = false;
		f.draw_charges = false;
		f.absolute_xy_offset = this_tree_entry.get_absolute_pos() - context.get_tree_entry(location).get_absolute_pos();

		item_button::draw_proc(context, dereferenced, in, f);
	}
}

void hotbar_button::advance_elements(const logic_gui_context& context, const this_in_item& this_id, const gui_entropy& entropies, const augs::delta dt) {
	base::advance_elements(context, this_id, entropies, dt);

	for (const auto& info : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(info);

		if (info.is_ldown_or_double_or_triple()) {

		}

		if (info.msg == gui_event::hover) {
			this_id->elapsed_hover_time_ms = 0.f;
		}
	}

	if (this_id->detector.is_hovered) {
		this_id->elapsed_hover_time_ms += dt.in_milliseconds();
	}
	
	if (this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		this_id->elapsed_hover_time_ms = this_id->hover_highlight_duration_ms;
	}
}

void hotbar_button::rebuild_layouts(const logic_gui_context& context, const this_in_item& this_id) {
	base::rebuild_layouts(context, this_id);
}