#include "dx_button.h"

dx_button::dx_button() {
	corners.lt_texture = assets::texture_id::HOTBAR_BUTTON_LT;
	border_corners.lt_texture = assets::texture_id::HOTBAR_BUTTON_LT_BORDER;
}

vec2i dx_button::get_target_button_size() const {
	return corners.internal_size_to_cornered_size(get_text_bbox(appearing_caption.get_total_target_text(), 0)) - vec2i(0, 3);
}

void dx_button::set_appearing_caption(const augs::gui::text::fstr text) {
	appearing_caption.population_interval = 100.f;

	appearing_caption.should_disappear = false;
	appearing_caption.target_text[0] = text;
}