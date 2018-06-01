#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/transcendental/step_declaration.h"

struct wielding_result {
	enum class type {
		NO_SPACE_FOR_HOLSTER,
		NOT_ENOUGH_HANDS,
		THE_SAME_SETUP,
		SUCCESSFUL
	};

	type result = type::THE_SAME_SETUP;
	augs::constant_size_vector<item_slot_transfer_request, 4> transfers;

	void apply(const logic_step);

	void play_effects_only_in_first();
	void play_effects_only_in_last();

	bool successful() const {
		return result == type::SUCCESSFUL;
	}
};
