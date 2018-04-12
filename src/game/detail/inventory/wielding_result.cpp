#include "wielding_result.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"

void wielding_result::apply(const logic_step step) {
	ensure_eq(result, type::SUCCESSFUL);

	perform_transfers(transfers, step);
}