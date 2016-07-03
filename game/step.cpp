#include "step.h"
#include "game/systems/item_system.h"
#include "cosmos.h"

basic_viewing_step::basic_viewing_step(const cosmos& cosm, augs::variable_delta delta, augs::renderer& renderer) : cosm(cosm), delta(delta), renderer(renderer) {}

augs::variable_delta basic_viewing_step::get_delta() const {
	return delta;
}

viewing_step::viewing_step(basic_viewing_step basic_step, state_for_drawing_camera camera_state) : basic_viewing_step(basic_step), camera_state(camera_state) {

}

vec2 viewing_step::get_screen_space(vec2 pos) const {
	return pos - camera_state.transformed_visible_world_area_aabb.get_position();
}

fixed_step::fixed_step(cosmos& cosm, augs::machine_entropy entropy) : cosm(cosm), entropy(entropy) {}

augs::fixed_delta fixed_step::get_delta() const {
	return cosm.delta;
}