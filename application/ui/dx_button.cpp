#include <tuple>
#include "dx_button.h"
#include "game/resources/manager.h"

dx_button::dx_button() {
	corners.inside_texture = assets::texture_id::MENU_BUTTON_INSIDE;

	auto& manager = get_resource_manager();

	click_sound.bind_buffer(manager.find(assets::sound_buffer_id::BUTTON_CLICK)->get_variation(0).request_original());
	hover_sound.bind_buffer(manager.find(assets::sound_buffer_id::BUTTON_HOVER)->get_variation(0).request_original());
}

vec2i dx_button::get_target_button_size() const {
	return corners.internal_size_to_cornered_size(get_text_bbox(appearing_caption.get_total_target_text(), 0)) - vec2i(0, 3);
}

void dx_button::set_appearing_caption(const augs::gui::text::fstr text) {
	appearing_caption.population_interval = 100.f;

	appearing_caption.should_disappear = false;
	appearing_caption.target_text[0] = text;
}