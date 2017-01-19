#include "step.h"
#include "game/systems_stateless/item_system.h"
#include "cosmos.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

augs::variable_delta viewing_step::get_delta() const {
	return delta;
}

double viewing_step::get_interpolated_total_time_passed_in_seconds() const {
	return cosm.get_total_time_passed_in_seconds() + get_delta().view_interpolation_ratio() * get_delta().in_seconds();
}

viewing_step::viewing_step(
	const cosmos& cosm, 
	viewing_session& session,
	const augs::variable_delta& delta,
	augs::renderer& renderer, 
	const camera_cone camera,
	const entity_id viewed_character,
	const visible_entities& visible
) : 
	const_cosmic_step(cosm), 
	session(session),
	delta(delta), 
	renderer(renderer), 
	camera(camera),
	viewed_character(viewed_character),
	visible(visible) 
{
}

vec2 viewing_step::get_screen_space(const vec2 pos) const {
	return pos - camera.get_transformed_visible_world_area_aabb().get_position();
}