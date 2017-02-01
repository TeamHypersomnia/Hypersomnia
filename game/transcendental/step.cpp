#include "step.h"
#include "game/systems_stateless/item_system.h"
#include "cosmos.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

double viewing_step::get_interpolated_total_time_passed_in_seconds() const {
	return cosm.get_total_time_passed_in_seconds() + get_interpolation_ratio() * cosm.get_fixed_delta().in_seconds();
}

float viewing_step::get_interpolation_ratio() const {
	return interpolation_ratio;
}

viewing_step::viewing_step(
	const config_lua_table& config,
	const cosmos& cosm,
	const viewing_session& session,
	const float interpolation_ratio,
	augs::renderer& renderer, 
	const camera_cone camera,
	const entity_id viewed_character,
	const visible_entities& visible
) : 
	const_cosmic_step(cosm),
	config(config),
	session(session),
	interpolation_ratio(interpolation_ratio),
	renderer(renderer), 
	camera(camera),
	viewed_character(viewed_character),
	visible(visible) 
{}

vec2 viewing_step::get_screen_space(const vec2 pos) const {
	return pos - camera.get_transformed_visible_world_area_aabb().get_position();
}