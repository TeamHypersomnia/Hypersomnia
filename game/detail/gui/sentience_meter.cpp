#include "drag_and_drop_target_drop_item.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/item_component.h"
#include "game/detail/gui/root_of_inventory_gui.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/systems_audiovisual/gui_element_system.h"
#include "augs/tweaker.h"

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

	auto mat = this_id->get_icon_mat(this_id);

	if (this_id->detector.is_hovered) {
		mat.color.a = 255;
	}
	else {
		mat.color.a = 120;
	}

	draw_centered_texture(
		context, 
		this_id, 
		info, 
		mat
	);
}

void sentience_meter::advance_elements(const game_gui_context context, const this_pointer this_id, const gui_entropy& entropies, const augs::delta) {
	for (const auto& e : entropies.get_events_for(this_id)) {
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

void sentience_meter::rebuild_layouts(
	const game_gui_context context, 
	const this_pointer this_id
) {
	const auto idx = static_cast<int>(this_id.get_location().type);
	const auto screen_size = context.get_character_gui().get_screen_size();
	const auto icon_size = (*this_id->get_icon_mat(this_id).tex).get_size();

	const auto lt = vec2i(screen_size.x - 200, 250 + idx * (icon_size.y + 4));
	
	this_id->rc.set_position(lt);
	this_id->rc.set_size(icon_size);
}