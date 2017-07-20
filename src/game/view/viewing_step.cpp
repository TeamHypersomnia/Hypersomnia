#include "viewing_step.h"
#include "game/transcendental/cosmos.h"

double viewing_step::get_interpolated_total_time_passed_in_seconds() const {
	return cosm.get_total_time_passed_in_seconds(get_interpolation_ratio());
}

double viewing_step::get_interpolation_ratio() const {
	return interpolation_ratio;
}

viewing_step::viewing_step(
	const cosmos& cosm,
	const audiovisual_state& audiovisuals,
	const game_drawing_settings drawing,
	const hotbar_settings hotbar,
	const input_context input_information,
	const double interpolation_ratio,
	augs::renderer& renderer,
	const camera_cone camera,
	const entity_id viewed_character,
	const visible_entities& visible,
	const bool is_ingame_menu_active
) :
	const_cosmic_step(cosm),
	audiovisuals(audiovisuals),
	input_information(input_information),
	drawing(drawing),
	interpolation_ratio(interpolation_ratio),
	renderer(renderer),
	camera(camera),
	viewed_character(viewed_character),
	visible(visible),
	is_ingame_menu_active(is_ingame_menu_active)
{}