#include "wielding_result.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"

void wielding_result::apply(const logic_step step) {
	ensure_eq(result, type::SUCCESSFUL);
	perform_transfers(transfers, step);
}

void wielding_result::play_effects_only_in_first() {
	for (auto& r : transfers) {
		r.play_transfer_sounds = false;
		r.play_transfer_particles = false;
	}
	
	if (transfers.size() > 0) {
		transfers.front().play_transfer_particles = true;
		transfers.front().play_transfer_sounds = true;
	}
}

void wielding_result::play_effects_only_in_last() {
	for (auto& r : transfers) {
		r.play_transfer_sounds = false;
		r.play_transfer_particles = false;
	}
	
	if (transfers.size() > 0) {
		transfers.back().play_transfer_particles = true;
		transfers.back().play_transfer_sounds = true;
	}
}