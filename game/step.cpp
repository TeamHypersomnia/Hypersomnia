#include "step.h"
#include "game/systems/item_system.h"
#include "cosmos.h"

variable_step::variable_step(const cosmos& cosm, augs::variable_delta delta) : cosm(cosm), delta(delta) {}

fixed_step::fixed_step(cosmos& cosm, augs::machine_entropy entropy) : cosm(cosm), entropy(entropy) {}

augs::variable_delta variable_step::get_delta() const {
	return delta;
}

augs::fixed_delta fixed_step::get_delta() const {
	return cosm.delta;
}