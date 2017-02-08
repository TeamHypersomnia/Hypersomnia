#include "drag_and_drop_target_drop_item.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/sentience_component.h"
#include "game/detail/gui/root_of_inventory_gui.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/systems_audiovisual/gui_element_system.h"
#include "augs/tweaker.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/stroke.h"

perk_meter::perk_meter() {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING);
}

void perk_meter::draw(
	const viewing_game_gui_context context,
	const const_this_pointer this_id,
	const augs::gui::draw_info info
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	const auto gui_element_entity = context.get_gui_element_entity();
	const auto& cosmos = context.get_cosmos();

	auto icon_mat = this_id->get_icon_mat(this_id);

	if (this_id->detector.is_hovered) {
		icon_mat.color.a = 255;
	}
	else {
		icon_mat.color.a = 200;
	}

	const auto absolute = context.get_tree_entry(this_id).get_absolute_rect();

	auto icon_rect = absolute;
	icon_rect.set_size((*icon_mat.tex).get_size());

	draw_clipped_rect(
		icon_mat,
		icon_rect,
		context,
		context.get_tree_entry(this_id).get_parent(),
		info.v
	);

	const auto total_spacing = this_id->get_total_spacing();

	{
		const auto full_bar_rect_bordered = this_id->get_full_value_bar_rect_bordered(context, this_id, absolute);
		const auto full_bar_rect = this_id->get_full_value_bar_rect(context, this_id, absolute);

		auto bar_mat = this_id->get_bar_mat(this_id);
		bar_mat.color.a = icon_mat.color.a;

		const auto this_type = this_id.get_location().type;

		const auto& sentience = gui_element_entity.get<components::sentience>();
		const auto dt = cosmos.get_fixed_delta();
		const auto now = cosmos.get_timestamp();

		const auto ratio = sentience.call_on(this_type, [&](const auto& m) { return m.get_ratio(now, dt); });
		auto actual_bar_rect = full_bar_rect;
		const auto bar_width = static_cast<int>(actual_bar_rect.w() * ratio);
		actual_bar_rect.w(bar_width);

		draw_clipped_rect(
			bar_mat,
			actual_bar_rect,
			context,
			context.get_tree_entry(this_id).get_parent(),
			info.v
		);

		augs::gui::solid_stroke stroke;
		stroke.set_material(bar_mat);
		stroke.set_width(this_id->border_width);
		stroke.draw(info.v, full_bar_rect, ltrb(), this_id->border_spacing);

		if (bar_width >= 1) {
			for (const auto& p : this_id->particles) {
				auto particle_mat = p.mat;
				particle_mat.color = bar_mat.color + rgba(30, 30, 30, 0);

				const auto particle_rect = ltrb(full_bar_rect.get_position() - vec2(6, 6) + p.relative_pos, (*particle_mat.tex).get_size());

				draw_clipped_rect(
					particle_mat,
					particle_rect,
					actual_bar_rect,
					info.v
				);
			}
		}
	}
}

ltrb perk_meter::get_full_value_bar_rect_bordered(
	const const_game_gui_context context,
	const const_this_pointer this_id,
	const ltrb absolute
) const {
	auto icon_rect = absolute;

	auto icon_mat = this_id->get_icon_mat(this_id);
	icon_rect.set_size((*icon_mat.tex).get_size());
	augs::gui::text_drawer drawer;
	drawer.set_text(augs::gui::text::format(L"99999", assets::font_id::GUI_FONT));

	const auto max_value_caption_size = drawer.get_bbox();

	auto full_bar_rect = icon_rect;
	full_bar_rect.set_position(icon_rect.get_position() + vec2i(get_total_spacing() + icon_rect.get_size().x, 0));
	full_bar_rect.r = absolute.r - max_value_caption_size.x;

	return full_bar_rect;
}

ltrb perk_meter::get_full_value_bar_rect(
	const const_game_gui_context context,
	const const_this_pointer this_id,
	const ltrb absolute
) const {
	return get_full_value_bar_rect_bordered(context, this_id, absolute).expand_from_center({ static_cast<float>(-get_total_spacing()), static_cast<float>(-get_total_spacing()) });
}


void perk_meter::advance_elements(
	const game_gui_context context,
	const this_pointer this_id,
	const augs::delta dt
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	this_id->seconds_accumulated += dt.in_seconds();

	if (this_id->particles.size() > 0) {
		randomization rng(static_cast<int>(this_id.get_location().type) + context.get_cosmos().get_total_time_passed_in_seconds() * 1000);

		const auto bar_size = this_id->get_full_value_bar_rect(context, this_id, this_id->rc).get_size();

		while (this_id->seconds_accumulated > 0.f) {
			for (auto& p : this_id->particles) {
				const auto action = rng.randval(0, 8);

				if (action == 0) {
					const auto y_dir = rng.randval(0, 1);

					if (y_dir == 0) {
						++p.relative_pos.y;
					}
					else {
						--p.relative_pos.y;
					}
				}

				++p.relative_pos.x;

				p.relative_pos.y = std::max(0, p.relative_pos.y);
				p.relative_pos.x = std::max(0, p.relative_pos.x);

				p.relative_pos.x %= static_cast<int>(bar_size.x + 12);
				p.relative_pos.y %= static_cast<int>(bar_size.y + 12);
			}

			this_id->seconds_accumulated -= 1.f / 15;
		}
	}
}

void perk_meter::respond_to_events(
	const game_gui_context context,
	const this_pointer this_id,
	const gui_entropy& entropies
) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(e);
	}
}

augs::gui::material perk_meter::get_icon_mat(const const_this_pointer this_id) const {
	switch (this_id.get_location().type) {
	case perk_meter_type::HASTE: return{ assets::texture_id::PERK_HASTE_ICON, white };
	case perk_meter_type::ELECTRIC_SHIELD: return{ assets::texture_id::PERK_HASTE_ICON, white };
	default: ensure(false);  return{};
	}
}

augs::gui::material perk_meter::get_bar_mat(const const_this_pointer this_id) const {
	switch (this_id.get_location().type) {
	case perk_meter_type::HASTE: return{ assets::texture_id::BLANK, green - rgba(30, 30, 30, 0) };
	case perk_meter_type::ELECTRIC_SHIELD: return{ assets::texture_id::BLANK, vsblue - rgba(30, 30, 30, 0) };
	default: ensure(false);  return{};
	}
}

void perk_meter::rebuild_layouts(
	const game_gui_context context,
	const this_pointer this_id
) {
	const auto this_type = this_id.get_location().type;

	const auto gui_element_entity = context.get_gui_element_entity();
	const auto& cosmos = context.get_cosmos();

	const auto& sentience = gui_element_entity.get<components::sentience>();

	const auto dt = cosmos.get_fixed_delta();
	const auto now = cosmos.get_timestamp();

	if (!sentience.call_on(this_type, [&](const auto& m) {
		return m.is_enabled(now, dt);
	})) {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
		return;
	}
	else {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	unsigned vertical_index = 3;

	for (unsigned i = 0; i < static_cast<unsigned>(this_type); ++i) {
		if (sentience.call_on(static_cast<perk_meter_type>(i), [&](const auto& m) {
			return m.is_enabled(now, dt);
		})) {
			++vertical_index;
		}
	}

	const auto screen_size = context.get_character_gui().get_screen_size();
	const auto icon_size = (*this_id->get_icon_mat(this_id).tex).get_size();
	const auto with_bar_size = vec2i(icon_size.x + 4 + 180, icon_size.y);

	const auto lt = vec2i(screen_size.x - 220, 20 + vertical_index * (icon_size.y + 4));

	this_id->rc.set_position(lt);
	this_id->rc.set_size(with_bar_size);

	if (this_id->particles.empty()) {
		randomization rng(static_cast<int>(this_id.get_location().type) + 3);

		const auto bar_size = this_id->get_full_value_bar_rect(context, this_id, this_id->rc).get_size();

		for (size_t i = 0; i < 40; ++i) {
			const augs::gui::material mats[3] = {
				assets::texture_id::WANDERING_CROSS,
				assets::texture_id::BLINK_FIRST,
				static_cast<assets::texture_id>(static_cast<int>(assets::texture_id::BLINK_FIRST) + 2),
			};

			effect_particle new_part;
			new_part.relative_pos = rng.randval(vec2(0, 0), bar_size);
			new_part.mat = mats[rng.randval(0, 2)];

			this_id->particles.push_back(new_part);
		}
	}
}