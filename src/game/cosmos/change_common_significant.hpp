#pragma once
#include "augs/enums/callback_result.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmic_functions.h"

template <class F>
void cosmos::change_common_significant(F&& callback) {
	auto status = changer_callback_result::INVALID;
	auto& self = *this;

	auto refresh_when_done = augs::scope_guard([&]() {
		if (status != changer_callback_result::DONT_REFRESH) {
			/*	
				Always first reinfer the common,
				only later the entities, as they might use the common inferred during their own reinference. 
			*/
			common.reinfer();
			cosmic::reinfer_all_entities(self);
		}
	});

	status = callback(common.significant);
}
