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

#include "augs/gui/stroke.h"

sentience_meter::sentience_meter() {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING);
}

void sentience_meter::draw(
	const viewing_game_gui_context context, 
	const const_this_pointer this_id, 
	const augs::gui::draw_info info
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

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

	const auto border_width = 1;
	const auto border_spacing = 1;
	const auto total_spacing = border_width + border_spacing;

	{
		auto full_bar_rect = icon_rect;
		full_bar_rect.set_position(icon_rect.get_position() + vec2i(total_spacing + icon_rect.get_size().x, 0));
		full_bar_rect.r = absolute.r;
		full_bar_rect.expand_from_center({ -total_spacing, -total_spacing });

		auto bar_mat = this_id->get_bar_mat(this_id);
		bar_mat.color.a = icon_mat.color.a;

		const auto& sentience = context.get_gui_element_entity().get<components::sentience>();
		const auto ratio = sentience.get(this_id.get_location().type).ratio();
		auto actual_bar_rect = full_bar_rect;
		actual_bar_rect.w(actual_bar_rect.w() * ratio);

		draw_clipped_rect(
			bar_mat,
			actual_bar_rect,
			context,
			context.get_tree_entry(this_id).get_parent(),
			info.v
		);

		augs::gui::solid_stroke stroke;
		stroke.set_material(bar_mat);
		stroke.set_width(border_width);
		stroke.draw(info.v, full_bar_rect, ltrb(), border_spacing);
	}
}

void sentience_meter::advance_elements(
	const game_gui_context context, 
	const this_pointer this_id, 
	const gui_entropy& entropies, 
	const augs::delta
) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		LOG_NVPS(static_cast<int>(e.msg));
		this_id->detector.update_appearance(e);
	}
}

augs::gui::material sentience_meter::get_icon_mat(const const_this_pointer this_id) const {
	switch (this_id.get_location().type) {
	case sentience_meter_type::HEALTH: return{ assets::texture_id::HEALTH_ICON, white };
	case sentience_meter_type::CONSCIOUSNESS: return{ assets::texture_id::CONSCIOUSNESS_ICON, white };
	case sentience_meter_type::PERSONAL_ELECTRICITY: return{ assets::texture_id::PERSONAL_ELECTRICITY_ICON, white };
	default: ensure(false);  return{};
	}
}

augs::gui::material sentience_meter::get_bar_mat(const const_this_pointer this_id) const {
	switch (this_id.get_location().type) {
	case sentience_meter_type::HEALTH: return{ assets::texture_id::BLANK, red };
	case sentience_meter_type::CONSCIOUSNESS: return{ assets::texture_id::BLANK, orange };
	case sentience_meter_type::PERSONAL_ELECTRICITY: return{ assets::texture_id::BLANK, cyan };
	default: ensure(false);  return{};
	}
}

void sentience_meter::rebuild_layouts(
	const game_gui_context context, 
	const this_pointer this_id
) {
	const auto idx = static_cast<int>(this_id.get_location().type);
	const auto screen_size = context.get_character_gui().get_screen_size();
	const auto icon_size = (*this_id->get_icon_mat(this_id).tex).get_size();
	const auto with_bar_size = vec2i(icon_size.x + 4 + 180, icon_size.y);

	const auto lt = vec2i(screen_size.x - 220, 220 + idx * (icon_size.y + 4));
	
	this_id->rc.set_position(lt);
	this_id->rc.set_size(with_bar_size);
}