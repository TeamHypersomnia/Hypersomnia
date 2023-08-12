#include "augs/drawing/drawing.hpp"
#include "augs/templates/container_templates.h"
#include "hotbar_button.h"
#include "augs/gui/button_corners.h"
#include "game/cosmos/cosmos.h"
#include "view/game_gui/elements/item_button.h"
#include "game/components/item_component.h"
#include "view/game_gui/elements/character_gui.h"
#include "augs/gui/button_corners.h"
#include "augs/gui/text/printer.h"
#include "view/game_gui/elements/drag_and_drop.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "view/game_gui/game_gui_system.h"

#include "game/detail/gun/ammo_logic.h"

#include "view/game_gui/elements/hotbar_settings.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

using namespace augs::gui::text;
using namespace augs::gui;

bool hotbar_button::is_assigned(const const_entity_handle item) const {
	if (item.dead()) {
		return false;
	}

	return 
		last_assigned    == last_assigned_type(item.get_id()) 
		|| last_assigned == last_assigned_type(item.get_flavour_id())
	;
}

const_entity_handle hotbar_button::get_assigned_entity(const const_entity_handle owner_transfer_capability, int* const out_stackable_count, int offset) const {
	return std::visit(
		[&]<typename A>(const A& assigned) {
			const auto& cosm = owner_transfer_capability.get_cosmos();
			if constexpr(std::is_same_v<A, entity_id>) {
				const auto handle = cosm[assigned];

				if (handle.alive()) {
					if (handle.get_owning_transfer_capability() == owner_transfer_capability) {
						const auto slot = handle.get_current_slot();
						const bool should_ignore = slot->is_mounted_slot() && slot.get_type() != slot_function::TORSO_ARMOR;

						if (!should_ignore) {
							return handle;
						}
					}
				}

				return cosm[entity_id()];
			}
			else {
				entity_id found;

				owner_transfer_capability.for_each_contained_item_recursive(
					[&](const auto& item) {
						if (item.get_flavour_id() == assigned) {
							found = item.get_id();

							if (offset > 0) {
								--offset;
								return recursive_callback_result::CONTINUE_AND_RECURSE;
							}

							if (out_stackable_count) {
								++(*out_stackable_count);
								return recursive_callback_result::CONTINUE_AND_RECURSE;
							}

							return recursive_callback_result::ABORT;
						}

						return recursive_callback_result::CONTINUE_AND_RECURSE;
					}
				);

				return cosm[found];
			}
		},
		last_assigned
	);
}

bool hotbar_button::is_selected_as_primary(const const_entity_handle owner_transfer_capability) const {
	const auto actually_owned = get_assigned_entity(owner_transfer_capability).alive();

	return actually_owned && is_assigned(owner_transfer_capability.get_if_any_item_in_hand_no(0));
}

bool hotbar_button::is_selected_as_secondary(const const_entity_handle owner_transfer_capability) const {
	const auto actually_owned = get_assigned_entity(owner_transfer_capability).alive();

	return actually_owned && is_assigned(owner_transfer_capability.get_if_any_item_in_hand_no(1));
}

void hotbar_button::clear_assigned() {
	last_assigned = entity_id();
}

button_corners_info hotbar_button::get_button_corners_info() const {
	button_corners_info corners;
	corners.inside_texture = assets::necessary_image_id::HOTBAR_BUTTON;
	corners.flip = flip_flags::make_horizontally();

	return corners;
}

vec2i hotbar_button::get_bbox(
	const necessary_images_in_atlas_map& necessarys,
	const image_definitions_map& defs,
	const const_entity_handle owner_transfer_capability
) const {
	const auto ent = get_assigned_entity(owner_transfer_capability);

	if (ent.dead()) {
		return { 45, 55 };
	}

	return get_button_corners_info().internal_size_to_cornered_size(
		necessarys,
		item_button::calc_button_layout(ent, defs, true).get_size()
	);
}

void hotbar_button::draw(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}
	
	if (!context.dependencies.settings.draw_hotbar) {
		return;
	}

	const auto& this_tree_entry = context.get_tree_entry(this_id);
	const auto owner_transfer_capability = context.get_subject_entity();
	const auto settings = context.get_hotbar_settings();
	const auto& image_defs = context.get_image_metas();
	const auto& necessarys = context.get_necessary_images();
	const auto& gui_font = context.get_gui_font();
	const auto output = context.get_output();

	auto absolute_rc = this_tree_entry.get_absolute_rect();

	constexpr int left_rc_spacing = 2;
	constexpr int right_rc_spacing = 1;

	absolute_rc.l += left_rc_spacing;
	absolute_rc.r -= right_rc_spacing;

	const auto corners = this_id->get_button_corners_info();
	
	const auto internal_rc = corners.cornered_rc_to_internal_rc(necessarys, absolute_rc);
	
	int stackable_count = 0;

	const auto assigned_entity = this_id->get_assigned_entity(owner_transfer_capability, &stackable_count);
	const bool has_assigned_entity = assigned_entity.alive();

	const bool is_in_primary = this_id->is_selected_as_primary(owner_transfer_capability);
	const bool is_in_secondary = this_id->is_selected_as_secondary(owner_transfer_capability);

	const bool is_assigned_entity_selected = is_in_primary || is_in_secondary;

	const auto& detector = this_id->detector;

	auto colorize = cyan;

	rgba distinguished_border_color = cyan;

	if (is_in_primary) {
		distinguished_border_color = settings.primary_selected_color;
	}
	else if (is_in_secondary) {
		distinguished_border_color = settings.secondary_selected_color;
	}

	if (settings.colorize_inside_when_selected) {
		colorize = distinguished_border_color;
	}

	auto distinguished_border_function = is_button_border;

	rgba inside_col = white;
	rgba border_col = white;

	inside_col.a = 20;

	if (has_assigned_entity) {
		inside_col.a += 10;
	}

	if (settings.increase_inside_alpha_when_selected && is_assigned_entity_selected) {
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

	if (has_assigned_entity) {
		if (is_ammo_depleted(assigned_entity, owner_transfer_capability)) {
			colorize = red;
			border_col = red;
			distinguished_border_color = red;
		}
	}

	inside_col *= colorize;
	border_col *= colorize;

	const auto label_style = style(gui_font, distinguished_border_color);

	std::array<bool, static_cast<size_t>(button_corner_type::COUNT)> visible_parts;
	std::fill(visible_parts.begin(), visible_parts.end(), false);

	auto part_visibility_flag = [&visible_parts](const button_corner_type n) -> bool& {
		return visible_parts[static_cast<size_t>(n)];
	};

	part_visibility_flag(button_corner_type::INSIDE) = true;

	part_visibility_flag(button_corner_type::L) = true;
	part_visibility_flag(button_corner_type::T) = true;
	part_visibility_flag(button_corner_type::R) = true;
	part_visibility_flag(button_corner_type::B) = true;

	part_visibility_flag(button_corner_type::LT) = true;
	part_visibility_flag(button_corner_type::RT) = true;
	part_visibility_flag(button_corner_type::RB) = true;
	part_visibility_flag(button_corner_type::LB) = true;

	part_visibility_flag(button_corner_type::LB_INTERNAL_BORDER) = true;
	part_visibility_flag(button_corner_type::RT_INTERNAL_BORDER) = true;

	if (has_assigned_entity) {
		part_visibility_flag(button_corner_type::LB_COMPLEMENT_BORDER) = true;
		part_visibility_flag(button_corner_type::LT_INTERNAL_BORDER) = true;
		part_visibility_flag(button_corner_type::RT_BORDER) = true;
	}

	if (is_assigned_entity_selected) {
		part_visibility_flag(button_corner_type::LT_BORDER) = true;
		part_visibility_flag(button_corner_type::RB_BORDER) = true;
		part_visibility_flag(button_corner_type::RB_INTERNAL_BORDER) = true;
		part_visibility_flag(button_corner_type::LB_BORDER) = true;

		part_visibility_flag(button_corner_type::L_BORDER) = true;
		part_visibility_flag(button_corner_type::T_BORDER) = true;
		part_visibility_flag(button_corner_type::R_BORDER) = true;
		part_visibility_flag(button_corner_type::B_BORDER) = true;
	}
	
	{
		corners.for_each_button_corner(
			necessarys,
			internal_rc, 
			[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
				auto col = is_button_border(type) ? border_col : inside_col;
				
				if (distinguished_border_function(type)) {
					col = distinguished_border_color;
				}

				if (part_visibility_flag(type)) {
					if (type != button_corner_type::RB_INTERNAL_BORDER && type != button_corner_type::RB_BORDER) {
						output.aabb(necessarys.at(id), drawn_rc, col, corners.flip);
					}
				}
				
				if (type == button_corner_type::LB_COMPLEMENT) {
					const auto intent_for_this_button = 
						static_cast<inventory_gui_intent_type>(
							static_cast<int>(
								inventory_gui_intent_type::HOTBAR_0
							) + this_id.get_location().index
						)
					;

					if (
						const auto bound_key = key_or_default(
							context.get_input_information(),
							intent_for_this_button
						);
						bound_key != augs::event::keys::key::INVALID
					) {
						const auto label_text = formatted_string{
							key_to_string(bound_key).substr(0, 1),
							label_style
						};

						const auto label_bbox = get_text_bbox(label_text);

						print_stroked(
							output,
							drawn_rc.right_bottom() - label_bbox,
							label_text
						);
					}
				}
			}
		);

		if (this_id->detector.is_hovered) {
			auto hover_effect_rc = internal_rc;

			part_visibility_flag(button_corner_type::RB_BORDER) = false;
			part_visibility_flag(button_corner_type::RB_INTERNAL_BORDER) = false;

			if (pushed) {
				const auto distance = 4.f;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				corners.for_each_button_corner(
					necessarys,
					hover_effect_rc, 
					[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
						if (part_visibility_flag(type) && is_button_border(type)) {
							auto col = colorize;

							if (distinguished_border_function(type)) {
								col = distinguished_border_color;
							}

							output.aabb(necessarys.at(id), drawn_rc, col, corners.flip);
						}
					}
				);
			}
			else {
				part_visibility_flag(button_corner_type::L_BORDER) = false;
				part_visibility_flag(button_corner_type::T_BORDER) = false;
				part_visibility_flag(button_corner_type::R_BORDER) = false;
				part_visibility_flag(button_corner_type::B_BORDER) = false;

				const auto max_duration = this_id->hover_highlight_duration_ms;
				const auto max_distance = this_id->hover_highlight_maximum_distance;
		
				const auto distance = (1.f - std::min(max_duration, this_id->elapsed_hover_time_ms) / max_duration) * max_distance;
				hover_effect_rc.expand_from_center(vec2(distance, distance));
		
				corners.for_each_button_corner(
					necessarys, 
					hover_effect_rc,
					[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
						if (part_visibility_flag(type) && is_button_border(type)) {
							auto col = colorize;

							if (distinguished_border_function(type)) {
								col = distinguished_border_color;
							}

							output.aabb(necessarys.at(id), drawn_rc, col, corners.flip);
						}
					}
				);
			}
		}
	}


	if (has_assigned_entity) {
		ensure(assigned_entity.has<components::item>());

		item_button_in_item location;
		location.item_id = assigned_entity.get_id();

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

		if (stackable_count > 1) {
			f.overridden_charge_count = stackable_count;
		}

		f.always_draw_charges_as_closed = true;
		f.draw_attachments_even_if_open = true;
		f.expand_size_to_grid = false;
		f.always_full_item_alpha = true;
		f.overridden_charges_pos = absolute_rc.right_top();

		const auto height_excess = absolute_rc.h() - this_id->get_bbox(necessarys, image_defs, owner_transfer_capability).y;
		
		f.absolute_xy_offset = internal_rc.get_position() - context.get_tree_entry(location).get_absolute_pos();
		f.absolute_xy_offset.y += height_excess / 2;
		f.absolute_xy_offset += vec2(4, 4);

		item_button::draw_proc(context, dereferenced, f);
	}
}

void hotbar_button::advance_elements(
	const game_gui_context context, 
	const this_in_item this_id, 
	const augs::delta dt
) {
	base::advance_elements(context, this_id, dt);

	if (this_id->detector.is_hovered) {
		this_id->elapsed_hover_time_ms += dt.in_milliseconds();
	}

	if (this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		this_id->elapsed_hover_time_ms = this_id->hover_highlight_duration_ms;
	}
}

void hotbar_button::respond_to_events(
	const game_gui_context context, 
	const this_in_item this_id, 
	const gui_entropy& entropies
) {
	base::respond_to_events(context, this_id, entropies);
	
	const auto& rect_world = context.get_rect_world();
	auto& gui = context.get_character_gui();

	for (const auto& info : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(info);

		if (info.msg == gui_event::lclick) {
			const auto subject = context.get_subject_entity();
			const auto assigned_entity = this_id->get_assigned_entity(subject);

			if (assigned_entity.alive()) {
				auto setup = wielding_setup::bare_hands();
				setup.hand_selections[0] = assigned_entity;

				//if (!setup.same_as_in(subject)) {
					context.get_game_gui_system().queue_wielding(subject, setup);
					//}
			}
		}

		if (info.msg == gui_event::hover) {
			this_id->elapsed_hover_time_ms = 0.f;
		}

		if (info == gui_event::lstarteddrag) {
			if (const auto assigned_entity = this_id->get_assigned_entity(context.get_subject_entity())) {
				if (const auto item = assigned_entity.find<components::item>()) {
					gui.dragged_charges = item->get_charges();
				}
			}
		}

		if (info.msg == gui_event::lfinisheddrag) {
			drag_and_drop_callback(context, prepare_drag_and_drop_result(context, this_id, rect_world.rect_hovered), info.total_dragged_amount);
		}
	}
}

void hotbar_button::rebuild_layouts(
	const game_gui_context context, 
	const this_in_item this_id
) {
	base::rebuild_layouts(context, this_id);
}