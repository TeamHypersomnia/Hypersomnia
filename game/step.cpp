#include "step.h"
#include "game/systems/item_system.h"
#include "cosmos.h"

basic_viewing_step::basic_viewing_step(const cosmos& cosm, augs::variable_delta delta, augs::renderer& renderer) : cosm(cosm), delta(delta), renderer(renderer) {}

augs::variable_delta basic_viewing_step::get_delta() const {
	return delta;
}

viewing_step::viewing_step(basic_viewing_step basic_step, const_entity_handle camera) : basic_viewing_step(basic_step), camera(camera) {

}

fixed_step::fixed_step(cosmos& cosm, augs::machine_entropy entropy) : cosm(cosm), entropy(entropy) {}

augs::fixed_delta fixed_step::get_delta() const {
	return cosm.delta;
}