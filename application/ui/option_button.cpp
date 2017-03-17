#include <tuple>
#include "option_button.h"
#include "game/resources/manager.h"

option_button::option_button() {
	corners.inside_texture = assets::game_image_id::MENU_BUTTON_INSIDE;

	auto& manager = get_resource_manager();

	click_sound.bind_buffer(manager.find(assets::sound_buffer_id::BUTTON_CLICK)->get_variation(0).request_original());
	hover_sound.bind_buffer(manager.find(assets::sound_buffer_id::BUTTON_HOVER)->get_variation(0).request_original());
}

vec2i option_button::get_target_button_size() const {
	return corners.internal_size_to_cornered_size(get_text_bbox(appearing_caption.get_total_target_text(), 0)) - vec2i(0, 3);
}

void option_button::set_appearing_caption(const augs::gui::text::formatted_string text) {
	appearing_caption.population_interval = 100.f;

	appearing_caption.should_disappear = false;
	appearing_caption.target_text[0] = text;
}