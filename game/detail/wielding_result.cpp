#include "wielding_result.h"
#include "game/detail/inventory_utils.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"

void wielding_result::apply(const logic_step step) {
	ensure(result == type::SUCCESSFUL);

	perform_transfers(transfers, step);
}