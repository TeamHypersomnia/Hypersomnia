#include "augs/ensure.h"
#include "augs/templates/string_templates.h"
#include "augs/drawing/drawing.h"
#include "augs/gui/text/printer.h"
#include "augs/graphics/renderer.h"

#include "game/detail/gui/item_button.h"
#include "game/detail/gui/pixel_line_connector.h"
#include "game/detail/gui/grid.h"
#include "game/detail/gui/game_gui_context.h"
#include "game/detail/gui/drag_and_drop.h"
#include "game/detail/gui/game_gui_root.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/enums/item_category.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/sprite_component.h"
#include "game/components/item_component.h"
#include "game/components/fixtures_component.h"
#include "game/systems_stateless/input_system.h"
#include "game/view/audiovisual_state/systems/game_gui_system.h"
#include "game/assets/all_assets.h"

#include "game/detail/gui/slot_button.h"

using namespace augs::gui::text;

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
		for (const auto& r : context.get_game_gui_system().pending_transfers) {
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

	draw_proc(context, this_id, f);
}

void item_button::draw_complete_with_children(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id
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

	draw_children(context, this_id);
	draw_proc(context, this_id, f);
}

void item_button::draw_grid_border_ghost(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
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

	draw_proc(context, this_id, f);
}

void item_button::draw_complete_dragged_ghost(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id, 
	const vec2 absolute_xy_offset
) {
	draw_dragged_ghost_inside(context, this_id, absolute_xy_offset);
}

item_button::layout_with_attachments item_button::calculate_button_layout(
	const const_entity_handle component_owner,
	const game_image_definitions& defs,
	const bool include_attachments
) {
	const auto& cosmos = component_owner.get_cosmos();
	
	layout_with_attachments output;
	output.push(component_owner.get_aabb(components::transform()));

	if (include_attachments) {
		component_owner.for_each_contained_item_recursive(
			[&](const const_entity_handle desc, const inventory_traversal& traversal) {
				if (traversal.item_remains_physical) {
					output.push(desc.get_aabb(traversal.attachment_offset));
				}

				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}
		);
	}
	
	const auto origin = output.aabb.left_top();

	for (auto& b : output.boxes) {
		b += -origin;
	}

	const auto flip = defs.at(component_owner.get<components::sprite>().tex).gui_usage.flip;

	if (flip.horizontally()) {
		for (auto& b : output.boxes) {
			const auto old_b = b;
			b.l = output.aabb.w() - old_b.r;
			b.r = output.aabb.w() - old_b.l;
		}
	}

	if (flip.vertically()) {
		for (auto& b : output.boxes) {
			const auto old_b = b;
			b.t = output.aabb.h() - old_b.b;
			b.b = output.aabb.h() - old_b.t;
		}
	}

	return output;
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
	const auto output = context.get_output();
	const auto& necessarys = context.get_necessary_images();
	auto& this_tree_entry = context.get_tree_entry(this_id);

	const auto former_absolute_pos = this_tree_entry.get_absolute_pos();
#if TODO
	this_tree_entry.set_absolute_pos(former_absolute_pos + f.absolute_xy_offset);
#endif
	const auto this_absolute_rect = context.get_tree_entry(this_id).get_absolute_rect();

	auto parent_slot = cosmos[item.get<components::item>().current_slot];

	rgba inside_col = cyan;
	rgba border_col = cyan;

	if (parent_slot->category_allowed != item_category::GENERAL) {
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
		output.gui_box_stretch_tex(context, this_id, inside_col);
	}

	if (f.draw_item) {
		{
			const bool draw_attachments = !this_id->is_container_open || f.draw_attachments_even_if_open;
			auto item_sprite = item.get<components::sprite>();
			const auto gui_def = context.get_game_image_definitions().at(item_sprite.tex).gui_usage;

			const auto layout = calculate_button_layout(item, context.get_game_image_definitions(), draw_attachments);
			
			vec2 expansion_offset;

			if (f.expand_size_to_grid) {
				const auto rounded_size = griddify_size(layout.aabb.get_size(), gui_def.bbox_expander);
				expansion_offset = (rounded_size - layout.aabb.get_size()) / 2;
			}

			const auto flip = gui_def.flip;
			item_sprite.flip = flip;

			item_sprite.color.a = 255;
			//item_sprite.color.a = border_col.a;

			if (f.always_full_item_alpha) {
				item_sprite.color.a = 255;
			}

			auto state = components::sprite::drawing_input{ augs::drawer (output) };

			const auto rc_pos = this_absolute_rect.get_position();
			state.renderable_transform.pos = layout.get_base_item_pos().center() + rc_pos + expansion_offset;

			if (draw_attachments) {
				size_t attachment_index = 1;

				const auto iteration_lambda = [&](const const_entity_handle desc, const inventory_traversal& trav) {
					const auto parent_slot = cosmos[desc.get<components::item>().current_slot];

					if (parent_slot.is_physically_connected_until(item)) {
						auto attachment_sprite = desc.get<components::sprite>();

						attachment_sprite.flip = flip;

						attachment_sprite.color.a = item_sprite.color.a;

						auto attachment_state = components::sprite::drawing_input(output);

						attachment_state.renderable_transform.pos = rc_pos + layout.boxes[attachment_index].center() + expansion_offset;
						attachment_state.renderable_transform.rotation = trav.attachment_offset.rotation;

						if (flip.horizontally()) {
							attachment_state.renderable_transform.flip_rotation();
						}

						if (flip.vertically()) {
							attachment_state.renderable_transform.flip_rotation();
						}

						attachment_sprite.draw(context.get_game_images(), attachment_state);

						++attachment_index;
					}

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				};

				item.for_each_contained_item_recursive(iteration_lambda);
			}

			item_sprite.draw(context.get_game_images(), state);
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
			else if (element.draw_space_available_inside_container_icons && item[slot_function::ITEM_DEPOSIT].alive()) {
				if (item.get<components::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE)) {
					if (!this_id->is_container_open) {
						printing_charge_count = true;
					}
				}

				if (printing_charge_count) {
					bottom_number_val = count_charges_in_deposit(item);
				}
				else {
					bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_real_space_available() / long double(SPACE_ATOMS_PER_UNIT);

					if (bottom_number_val < 1.0 && bottom_number_val > 0.0) {
						trim_zero = true;
					}

					label_color.rgb() = cyan.rgb();
				}

				//if (item[slot_function::ITEM_DEPOSIT]->category_allowed != item_category::GENERAL)
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

				const auto label_text = formatted_string {
					label_wstr, { context.get_gui_font(), label_color }
				};

				const auto label_bbox = get_text_bbox(label_text);

				print_stroked(
					output,
					context.get_tree_entry(this_id).get_absolute_rect().right_bottom() - label_bbox,
					label_text
				);
			}
		}
	}

	if (f.draw_border) {
		output.border(this_absolute_rect, border_col);
	}

	if (f.draw_connector && parent_slot.get_container().get_owning_transfer_capability() != parent_slot.get_container()) {
		draw_pixel_line_connector(
			output, 
			this_absolute_rect, 
			context.get_tree_entry(item_button_in_item{ parent_slot.get_container() }).get_absolute_rect(),
			border_col
		);
	}

	if (f.draw_container_opened_mark) {
		if (item.find<components::container>()) {
			assets::necessary_image_id container_icon;

			if (this_id->is_container_open) {
				container_icon = assets::necessary_image_id::CONTAINER_OPEN_ICON;
			}
			else {
				container_icon = assets::necessary_image_id::CONTAINER_CLOSED_ICON;
			}

			const auto size = necessarys.at(container_icon).get_size();

			output.aabb_lt(necessarys.at(container_icon), vec2(this_absolute_rect.r - size.x + 2, this_absolute_rect.t + 1), border_col);
		}
	}

#if TODO
	this_tree_entry.set_absolute_pos(former_absolute_pos);
#endif
}

bool item_button::is_inventory_root(const const_game_gui_context context, const const_this_in_item this_id) {
	const bool result = this_id.get_location().item_id == context.get_subject_entity();
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

	const auto& manager = context.get_game_image_definitions();

	if (sprite) {
		vec2i rounded_size = calculate_button_layout(item, manager, !this_id->is_container_open).aabb.get_size();
		rounded_size = griddify_size(rounded_size, manager.at(item.get<components::sprite>().tex).gui_usage.bbox_expander);
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

			if (info == gui_event::lstarteddrag) {
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

void item_button::draw(
	const viewing_game_gui_context context, 
	const const_this_in_item this_id
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	if (!is_being_wholely_dragged_or_pending_finish(context, this_id)) {
		this_id->draw_complete_with_children(context, this_id);
	}
}