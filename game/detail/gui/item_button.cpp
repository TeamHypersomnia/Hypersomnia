#include "game/detail/gui/item_button.h"
#include "game/detail/gui/pixel_line_connector.h"
#include "game/detail/gui/grid.h"
#include "game/detail/gui/game_gui_context.h"
#include "game/detail/gui/drag_and_drop.h"
#include "game/detail/gui/root_of_inventory_gui.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/renderer.h"

#include "augs/gui/stroke.h"

#include "game/enums/item_category.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/sprite_component.h"
#include "game/components/item_component.h"
#include "game/systems_stateless/gui_system.h"
#include "game/systems_stateless/input_system.h"
#include "game/systems_audiovisual/gui_element_system.h"
#include "game/resources/manager.h"
#include "augs/graphics/drawers.h"

#include "game/detail/gui/slot_button.h"

#include "augs/templates/string_templates.h"
#include "augs/ensure.h"

bool item_button::is_being_wholely_dragged_or_pending_finish(
	const const_game_gui_context context, 
	const const_this_in_item this_id
) {
	const auto& rect_world = context.get_rect_world();
	const auto& element = context.get_character_gui();
	const auto& cosmos = context.get_cosmos();

	if (rect_world.is_currently_dragging(this_id)) {
		const bool is_drag_partial = element.dragged_charges < cosmos[this_id.get_location().item_id].get<components::item>().charges;
		return !is_drag_partial;
	}
	else {
		for (const auto& r : context.get_gui_element_system().pending_transfers) {
			if (r.item == this_id.get_location().item_id) {
				return true;
			}
		}
	}

	return false;
}

item_button::item_button(xywh rc) : base(rc) {
	unset_flag(augs::gui::flag::CLIP);
	unset_flag(augs::gui::flag::SCROLLABLE);
	unset_flag(augs::gui::flag::FOCUSABLE);
}

void item_button::draw_dragged_ghost_inside(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	const draw_info in, 
	const vec2 absolute_xy_offset
) {
	drawing_settings f;
	f.draw_background = true;
	f.draw_item = true;
	f.draw_border = false;
	f.draw_connector = false;
	f.decrease_alpha = true;
	f.decrease_border_alpha = false;
	f.draw_container_opened_mark = false;
	f.draw_charges = false;
	f.absolute_xy_offset = absolute_xy_offset;

	draw_proc(context, this_id, in, f);
}

void item_button::draw_complete_with_children(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	draw_info in
) {
	drawing_settings f;
	f.draw_background = true;
	f.draw_item = true;
	f.draw_border = true;
	f.draw_connector = true;
	f.decrease_alpha = false;
	f.decrease_border_alpha = false;
	f.draw_container_opened_mark = true;
	f.draw_charges = true;

	draw_children(context, this_id, in);
	draw_proc(context, this_id, in, f);
}

void item_button::draw_grid_border_ghost(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	const draw_info in, 
	const vec2 absolute_xy_offset
) {
	drawing_settings f;
	f.draw_background = false;
	f.draw_item = false;
	f.draw_border = true;
	f.draw_connector = false;
	f.decrease_alpha = true;
	f.decrease_border_alpha = true;
	f.draw_container_opened_mark = false;
	f.draw_charges = true;
	f.absolute_xy_offset = griddify(absolute_xy_offset);

	draw_proc(context, this_id, in, f);
}

void item_button::draw_complete_dragged_ghost(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	const draw_info in, 
	const vec2 absolute_xy_offset
) {
	draw_dragged_ghost_inside(context, this_id, in, absolute_xy_offset);
}

item_button::layout_with_attachments item_button::calculate_button_layout(
	const const_entity_handle component_owner,
	const bool include_attachments
) {
	const auto& cosmos = component_owner.get_cosmos();
	
	layout_with_attachments output;
	output.push(component_owner.get_aabb(components::transform()));

	if (include_attachments) {
		component_owner.for_each_contained_item_recursive([&](const const_entity_handle desc, const inventory_traversal& traversal) {
			if (traversal.item_remains_physical) {
				output.push(desc.get_aabb(traversal.attachment_offset));
			}
		});
	}
	
	const auto origin = output.aabb.left_top();

	for (auto& b : output.boxes) {
		b += -origin;
	}

	const auto gui_def = get_resource_manager().find(component_owner.get<components::sprite>().tex)->settings.gui;

	if (gui_def.flip_horizontally) {
		for (auto& b : output.boxes) {
			const auto old_b = b;
			b.l = output.aabb.w() - old_b.r;
			b.r = output.aabb.w() - old_b.l;
		}
	}

	if (gui_def.flip_vertically) {
		for (auto& b : output.boxes) {
			const auto old_b = b;
			b.t = output.aabb.h() - old_b.b;
			b.b = output.aabb.h() - old_b.t;
		}
	}

	return std::move(output);
}

vec2 item_button::griddify_size(const vec2 size, const vec2 expander) {
	vec2i rounded_size = size;
	rounded_size += expander;

	rounded_size += 22;
	rounded_size /= 11;
	rounded_size *= 11;

	return rounded_size;
}

void item_button::draw_proc(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	const draw_info in, 
	const drawing_settings& f
) {
	if (is_inventory_root(context, this_id)) {
		return;
	}

	const auto& cosmos = context.get_cosmos();
	const auto& item = cosmos[this_id.get_location().item_id];
	const auto& detector = this_id->detector;
	const auto& rect_world = context.get_rect_world();
	const auto& element = context.get_character_gui();
	auto& this_tree_entry = context.get_tree_entry(this_id);

	const auto former_absolute_pos = this_tree_entry.get_absolute_pos();
	this_tree_entry.set_absolute_pos(former_absolute_pos + f.absolute_xy_offset);
	const auto this_absolute_rect = context.get_tree_entry(this_id).get_absolute_rect();

	auto parent_slot = cosmos[item.get<components::item>().current_slot];

	rgba inside_col = cyan;
	rgba border_col = cyan;

	if (parent_slot->for_categorized_items_only) {
		border_col = pink;
		inside_col = violet;
	}

	inside_col.a = 20;
	border_col.a = 190;

	if (detector.is_hovered) {
		inside_col.a = 30;
		border_col.a = 220;
	}

	if (detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		inside_col.a = 60;
		border_col.a = 255;
	}

	if (f.decrease_alpha) {
		inside_col.a = 15;
	}

	if (f.decrease_border_alpha) {
		border_col = slightly_visible_white;
	}

	if (f.draw_background) {
		draw_stretched_texture(context, this_id, in, augs::gui::material(assets::texture_id::BLANK, inside_col));
	}

	if (f.draw_item) {
		{
			const bool draw_attachments = !this_id->is_container_open || f.draw_attachments_even_if_open;
			auto item_sprite = item.get<components::sprite>();
			const auto gui_def = get_resource_manager().find(item_sprite.tex)->settings.gui;

			const auto layout = calculate_button_layout(item, draw_attachments);
			
			vec2 expansion_offset;

			if (f.expand_size_to_grid) {
				const auto rounded_size = griddify_size(layout.aabb.get_size(), gui_def.bbox_expander);
				expansion_offset = (rounded_size - layout.aabb.get_size()) / 2;
			}

			const auto flip_horizontally = gui_def.flip_horizontally;
			const auto flip_vertically = gui_def.flip_vertically;
			item_sprite.flip_horizontally = flip_horizontally;
			item_sprite.flip_vertically = flip_vertically;

			item_sprite.color.a = 255;
			//item_sprite.color.a = border_col.a;

			if (f.always_full_item_alpha) {
				item_sprite.color.a = 255;
			}

			components::sprite::drawing_input state(in.v);

			const auto rc_pos = this_absolute_rect.get_position();
			state.renderable_transform.pos = layout.get_base_item_pos().center() + rc_pos + expansion_offset;

			if (draw_attachments) {
				size_t attachment_index = 1;

				const auto iteration_lambda = [&](const const_entity_handle desc, const inventory_traversal& trav) {
					const auto parent_slot = cosmos[desc.get<components::item>().current_slot];

					if (parent_slot.should_item_inside_keep_physical_body(item)) {
						auto attachment_sprite = desc.get<components::sprite>();

						attachment_sprite.flip_horizontally = flip_horizontally;
						attachment_sprite.flip_vertically = flip_vertically;

						attachment_sprite.color.a = item_sprite.color.a;

						components::sprite::drawing_input attachment_state(in.v);

						attachment_state.renderable_transform.pos = rc_pos + layout.boxes[attachment_index].center() + expansion_offset;
						attachment_state.renderable_transform.rotation = trav.attachment_offset.rotation;

						if (flip_horizontally) {
							attachment_state.renderable_transform.flip_rotation();
						}

						if (flip_vertically) {
							attachment_state.renderable_transform.flip_rotation();
						}

						attachment_sprite.draw(attachment_state);

						++attachment_index;
					}
				};

				item.for_each_contained_item_recursive(iteration_lambda);
			}

			item_sprite.draw(state);
		}

		if (f.draw_charges) {
			const auto& item_data = item.get<components::item>();

			int considered_charges = item_data.charges;

			if (rect_world.is_currently_dragging(this_id)) {
				considered_charges = item_data.charges - element.dragged_charges;
			}

			long double bottom_number_val = -1.f;
			bool printing_charge_count = false;
			bool trim_zero = false;

			auto label_color = border_col;

			if (considered_charges > 1) {
				bottom_number_val = considered_charges;
				printing_charge_count = true;
			}
			else if (element.draw_free_space_inside_container_icons && item[slot_function::ITEM_DEPOSIT].alive()) {
				if (item.get<components::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE)) {
					if (!this_id->is_container_open) {
						printing_charge_count = true;
					}
				}

				if (printing_charge_count) {
					bottom_number_val = count_charges_in_deposit(item);
				}
				else {
					bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_real_free_space() / long double(SPACE_ATOMS_PER_UNIT);

					if (bottom_number_val < 1.0 && bottom_number_val > 0.0) {
						trim_zero = true;
					}

					label_color.rgb() = cyan.rgb();
				}

				//if (item[slot_function::ITEM_DEPOSIT]->for_categorized_items_only)
				//	label_color.rgb() = pink.rgb();
				//else
				//	label_color.rgb() = cyan.rgb();
			}

			if (bottom_number_val > -1.f) {
				std::wstring label_wstr;

				if (printing_charge_count) {
					//label_wstr = L'x';
					label_color.rgb() = white.rgb();
					label_wstr += to_wstring(bottom_number_val);
				}
				else
					label_wstr = to_wstring(bottom_number_val, 2);

				if (trim_zero && label_wstr[0] == L'0') {
					label_wstr.erase(label_wstr.begin());
				}

				// else label_wstr = L'{' + label_wstr + L'}';

				auto bottom_number = augs::gui::text::format(label_wstr, augs::gui::text::style(assets::font_id::GUI_FONT, label_color));

				augs::gui::text_drawer charges_caption;
				charges_caption.set_text(bottom_number);
				charges_caption.bottom_right(context.get_tree_entry(this_id).get_absolute_rect());
				charges_caption.draw(in);
			}
		}
	}

	if (f.draw_border) {
		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material(assets::texture_id::BLANK, border_col));
		stroke.draw(in.v, this_absolute_rect);
	}

	if (f.draw_connector && parent_slot.get_container().get_owning_transfer_capability() != parent_slot.get_container()) {
		draw_pixel_line_connector(this_absolute_rect, context.get_tree_entry(item_button_in_item{ parent_slot.get_container() }).get_absolute_rect(), in, border_col);
	}

	if (f.draw_container_opened_mark) {
		if (item.find<components::container>()) {
			assets::texture_id container_icon;

			if (this_id->is_container_open) {
				container_icon = assets::texture_id::CONTAINER_OPEN_ICON;
			}
			else {
				container_icon = assets::texture_id::CONTAINER_CLOSED_ICON;
			}

			const auto size = assets::get_size(container_icon);

			augs::draw_rect(in.v, vec2(this_absolute_rect.r - size.x + 2, this_absolute_rect.t + 1), container_icon, border_col);
		}
	}

	this_tree_entry.set_absolute_pos(former_absolute_pos);
}

bool item_button::is_inventory_root(const const_game_gui_context context, const const_this_in_item this_id) {
	const bool result = this_id.get_location().item_id == context.get_gui_element_entity();
	ensure(!result);
	return result;
}

void item_button::rebuild_layouts(const game_gui_context context, const this_in_item this_id) {
	base::rebuild_layouts(context, this_id);

	const auto& cosmos = context.get_cosmos();
	const auto item = cosmos[this_id.get_location().item_id];

	if (is_inventory_root(context, this_id)) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
		this_id->set_flag(augs::gui::flag::DISABLE_HOVERING);
		return;
	}

	this_id->set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN, this_id->is_container_open && !is_being_wholely_dragged_or_pending_finish(context, this_id));
	this_id->set_flag(augs::gui::flag::DISABLE_HOVERING, is_being_wholely_dragged_or_pending_finish(context, this_id));

	vec2i parent_position;

	const auto* const sprite = item.find<components::sprite>();

	if (sprite) {
		vec2i rounded_size = calculate_button_layout(item, !this_id->is_container_open).aabb.get_size();
		rounded_size = griddify_size(rounded_size, get_resource_manager().find(item.get<components::sprite>().tex)->settings.gui.bbox_expander);
		this_id->rc.set_size(rounded_size);
	}

	auto parent_slot = cosmos[item.get<components::item>().current_slot];

	if (parent_slot->always_allow_exactly_one_item) {
		const_dereferenced_location<slot_button_in_container> parent_button = context.dereference_location(slot_button_in_container{ parent_slot.get_id() });

		this_id->rc.set_position(parent_button->rc.get_position());
	}
	else {
		this_id->rc.set_position(this_id->drag_offset_in_item_deposit);
	}
}

void item_button::respond_to_events(const game_gui_context context, const this_in_item this_id, const gui_entropy& entropies) {
	base::respond_to_events(context, this_id, entropies);

	const auto& cosmos = context.get_cosmos();
	const auto& item = cosmos[this_id.get_location().item_id];
	const auto& rect_world = context.get_rect_world();
	auto& element = context.get_character_gui();

	if (!is_inventory_root(context, this_id)) {
		for (const auto& info : entropies.get_events_for(this_id)) {
			this_id->detector.update_appearance(info);

			if (info == gui_event::ldrag) {
				element.dragged_charges = item.get<components::item>().charges;
			}

			if (info == gui_event::rclick) {
				this_id->is_container_open = !this_id->is_container_open;
			}

			if (info == gui_event::lfinisheddrag) {
				this_id->started_drag = false;

				drag_and_drop_callback(context, prepare_drag_and_drop_result(context, this_id, rect_world.rect_hovered), info.total_dragged_amount);
			}
		}
	}
}

void item_button::draw(const viewing_game_gui_context context, const const_this_in_item this_id, draw_info in) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	if (!is_being_wholely_dragged_or_pending_finish(context, this_id)) {
		this_id->draw_complete_with_children(context, this_id, in);
	}
}