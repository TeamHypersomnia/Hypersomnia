#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/transcendental/step_declaration.h"

struct wielding_result {
	enum class type {
		SOMETHING_WONT_FIT,
		THE_SAME_SETUP,
		SUCCESSFUL
	};

	type result = type::THE_SAME_SETUP;
	augs::constant_size_vector<item_slot_transfer_request_data, 4> transfers;

	void apply(const logic_step);

	bool successful() const {
		return result == type::SUCCESSFUL;
	}
};
