#include "hotbar_button.h"
#include "augs/gui/button_corners.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/gui/item_button.h"
#include "game/components/item_component.h"
#include "augs/gui/button_corners.h"

const_entity_handle hotbar_button::get_assigned_entity(const const_entity_handle owner_transfer_capability) const {
	const auto& cosm = owner_transfer_capability.get_cosmos();
	const auto handle = cosm[last_assigned_entity];

	if (handle.get_owning_transfer_capability() == owner_transfer_capability) {
		return handle;
	}

	return cosm[entity_id()];
}

entity_handle hotbar_button::get_assigned_entity(const entity_handle owner_transfer_capability) const {
	auto& cosm = owner_transfer_capability.get_cosmos();
	const auto handle = cosm[last_assigned_entity];

	if (handle.get_owning_transfer_capability() == owner_transfer_capability) {
		return handle;
	}

	return cosm[entity_id()];
}

button_corners_info hotbar_button::get_button_corners_info() const {
	button_corners_info corners;
	corners.lt_texture = assets::texture_id::HOTBAR_BUTTON_LT;
	corners.flip_horizontally = true;

	return corners;
}

vec2i hotbar_button::get_bbox(const const_entity_handle owner_transfer_capability) const {
	const auto ent = get_assigned_entity(owner_transfer_capability);

	if (ent.dead()) {
		return { 55, 55 };
	}

	return get_button_corners_info().internal_size_to_cornered_size(item_button::calculate_button_layout(ent, true).aabb.get_size());
}

void hotbar_button::draw(const viewing_gui_context& context, const const_this_in_item& this_id, draw_info in) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	const auto& rect_world = context.get_rect_world();
	const auto& this_tree_entry = context.get_tree_entry(this_id);
	auto absolute_rc = this_tree_entry.get_absolute_rect();
	const auto owner_transfer_capability = context.get_gui_element_entity();

	const int left_rc_spacing = 2;
	const int right_rc_spacing = 1;

	absolute_rc.l += left_rc_spacing;
	absolute_rc.r -= right_rc_spacing;

	const auto corners = this_id->get_button_corners_info();
	
	const bool flip = corners.flip_horizontally;

	const auto internal_rc = corners.cornered_rc_to_internal_rc(absolute_rc);
	
	const auto assigned_entity = this_id->get_assigned_entity(owner_transfer_capability);
	const bool has_assigned_entity = assigned_entity.alive();

	const bool is_in_primary = has_assigned_entity && owner_transfer_capability.wields_in_primary_hand(assigned_entity);
	const bool is_in_secondary = has_assigned_entity && owner_transfer_capability.wields_in_secondary_hand(assigned_entity);

	const bool is_assigned_entity_selected = is_in_primary || is_in_secondary;

	const auto& detector = this_id->detector;

	auto colorize = cyan;

	const bool colorize_background_when_selected = true;
	const bool increase_alpha_when_selected = false;

	rgba distinguished_border_color = cyan;

	if (is_in_primary) {
		distinguished_border_color = pink;
	}
	else if (is_in_secondary) {
		distinguished_border_color = vsblue;
	}

	if (colorize_background_when_selected) {
		colorize = distinguished_border_color;
	}

	auto distinguished_border_function = is_button_border;

	rgba inside_col = white;
	rgba border_col = white;

	inside_col.a = 20;

	if (has_assigned_entity) {
		inside_col.a += 10;
	}

	if (increase_alpha_when_selected && is_assigned_entity_selected) {
		inside_col.a += 20;
	}

	border_col.a = 190;

	if (detector.is_hovered) {
		inside_col.a += 10;
		border_col.a = 220;
	}

	const bool pushed = detector.current_appearance == augs::gui::appearance_detector::appearance::pushed;

	if (pushed) {
		inside_col.a += 40;
		border_col.a = 255;
	}

	inside_col *= colorize;
	border_col *= colorize;

	const auto label_style = augs::gui::text::style(assets::font_id::GUI_FONT, distinguished_border_color);

	const auto inside_mat = augs::gui::material(assets::texture_id::HOTBAR_BUTTON_INSIDE, inside_col);

	augs::gui::draw_clipped_rect(inside_mat, internal_rc, {}, in.v);

	std::array<bool, static_cast<size_t>(button_corner_type::COUNT)> visible_parts;
	std::fill(visible_parts.begin(), visible_parts.end(), false);

	visible_parts[static_cast<size_t>(button_corner_type::L)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::T)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::R)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::B)] = true;

	visible_parts[static_cast<size_t>(button_corner_type::LT)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::RT)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::RB)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::LB)] = true;

	visible_parts[static_cast<size_t>(button_corner_type::LB_INTERNAL_BORDER)] = true;
	visible_parts[static_cast<size_t>(button_corner_type::RT_INTERNAL_BORDER)] = true;

	if (has_assigned_entity) {
		visible_parts[static_cast<size_t>(button_corner_type::LB_COMPLEMENT_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::LT_INTERNAL_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::RB_INTERNAL_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::RT_BORDER)] = true;
	}

	if (is_assigned_entity_selected) {
		visible_parts[static_cast<size_t>(button_corner_type::LT_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::RB_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::LB_BORDER)] = true;

		visible_parts[static_cast<size_t>(button_corner_type::L_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::T_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::R_BORDER)] = true;
		visible_parts[static_cast<size_t>(button_corner_type::B_BORDER)] = true;
	}
	
	{
		corners.for_each_button_corner(internal_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
			auto col = is_button_border(type) ? border_col : inside_col;
			
			if (distinguished_border_function(type)) {
				col = distinguished_border_color;
			}

			if (visible_parts[static_cast<size_t>(type)]) {
				augs::gui::draw_clipped_rect(augs::gui::material(id, col), drawn_rc, {}, in.v, flip);
			}
			
			if (type == button_corner_type::LB_COMPLEMENT) {
				augs::gui::text_drawer number_caption;
				number_caption.set_text(augs::gui::text::format(typesafe_sprintf(L"%x", this_id.get_location().index), label_style));
				number_caption.bottom_right(drawn_rc);
				number_caption.draw_stroke(in.v);
				number_caption.draw(in.v);
			}
		});

		if (this_id->detector.is_hovered) {
			auto hover_effect_rc = internal_rc;

			visible_parts[static_cast<size_t>(button_corner_type::RB_BORDER)] = false;
			visible_parts[static_cast<size_t>(button_corner_type::RB_INTERNAL_BORDER)] = false;

			if (pushed) {
				const auto distance = 4.f;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				corners.for_each_button_corner(hover_effect_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
					if (visible_parts[static_cast<size_t>(type)] && is_button_border(type)) {
						auto col = colorize;

						if (distinguished_border_function(type)) {
							col = distinguished_border_color;
						}

						augs::gui::draw_clipped_rect(augs::gui::material(id, col), drawn_rc, {}, in.v, corners.flip_horizontally);
					}
				});
			}
			else {
				visible_parts[static_cast<size_t>(button_corner_type::L_BORDER)] = false;
				visible_parts[static_cast<size_t>(button_corner_type::T_BORDER)] = false;
				visible_parts[static_cast<size_t>(button_corner_type::R_BORDER)] = false;
				visible_parts[static_cast<size_t>(button_corner_type::B_BORDER)] = false;

				const auto max_duration = this_id->hover_highlight_duration_ms;
				const auto max_distance = this_id->hover_highlight_maximum_distance;
		
				const auto distance = (1.f - std::min(max_duration, this_id->elapsed_hover_time_ms) / max_duration) * max_distance;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				corners.for_each_button_corner(hover_effect_rc, [&](const button_corner_type type, const assets::texture_id id, const ltrb drawn_rc) {
					if (visible_parts[static_cast<size_t>(type)] && is_button_border(type)) {
						auto col = colorize;

						if (distinguished_border_function(type)) {
							col = distinguished_border_color;
						}

						augs::gui::draw_clipped_rect(augs::gui::material(id, col), drawn_rc, {}, in.v, corners.flip_horizontally);
					}
				});
			}
		}
	}


	if (has_assigned_entity) {
		ensure(assigned_entity.has<components::item>());

		item_button_in_item location;
		location.item_id = this_id->last_assigned_entity;

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
		f.draw_attachments_even_if_open = true;
		f.expand_size_to_grid = false;
		f.always_full_item_alpha = true;

		const auto height_excess = absolute_rc.h() - this_id->get_bbox(owner_transfer_capability).y;
		
		f.absolute_xy_offset = internal_rc.get_position() - context.get_tree_entry(location).get_absolute_pos();
		f.absolute_xy_offset.y += height_excess / 2;
		f.absolute_xy_offset += vec2(4, 4);

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