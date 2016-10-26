#include "step.h"
#include "game/systems_stateless/item_system.h"
#include "cosmos.h"

#include "game/transcendental/types_specification/all_messages_includes.h"

augs::variable_delta viewing_step::get_delta() const {
	return delta;
}

viewing_step::viewing_step(
	const cosmos& cosm, 
	const immediate_hud& hud,
	const aabb_highlighter& world_hover_highlighter,
	const augs::variable_delta& delta,
	augs::renderer& renderer, 
	state_for_drawing_camera camera_state) : cosm(cosm), hud(hud), world_hover_highlighter(world_hover_highlighter), delta(delta), renderer(renderer), camera_state(camera_state) {}

vec2 viewing_step::get_screen_space(vec2 pos) const {
	return pos - camera_state.transformed_visible_world_area_aabb.get_position();
}

fixed_step::fixed_step(cosmos& cosm, cosmic_entropy entropy) : cosm(cosm), entropy(entropy) {}

augs::fixed_delta fixed_step::get_delta() const {
	return cosm.get_fixed_delta();
}

cosmos& fixed_step::get_cosmos() {
	return cosm;
}