#include "wielding_result.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"

void wielding_result::apply(const logic_step step) {
	ensure_eq(result, type::SUCCESSFUL);
	perform_transfers(transfers, step);
}

void wielding_result::play_effects_only_in_first() {
	for (auto& r : transfers) {
		r.params.play_transfer_sounds = false;
		r.params.play_transfer_particles = false;
	}
	
	if (transfers.size() > 0) {
		transfers.front().params.play_transfer_particles = true;
		transfers.front().params.play_transfer_sounds = true;
	}
}

void wielding_result::play_effects_only_in_last() {
	for (auto& r : transfers) {
		r.params.play_transfer_sounds = false;
		r.params.play_transfer_particles = false;
	}
	
	if (transfers.size() > 0) {
		transfers.back().params.play_transfer_particles = true;
		transfers.back().params.play_transfer_sounds = true;
	}
}