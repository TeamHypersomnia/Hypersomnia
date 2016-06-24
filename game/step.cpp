#include "step.h"
#include "game/systems/item_system.h"
#include "cosmos.h"

augs::variable_delta variable_step::get_delta() const {
	return delta;
}

augs::fixed_delta fixed_step::get_delta() const {
	return cosm.delta;
}