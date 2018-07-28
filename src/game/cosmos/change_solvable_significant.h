#pragma once
#include "augs/enums/callback_result.h"

#include "augs/misc/scope_guard.h"
#include "game/cosmos/cosmic_functions.h"

template <class C, class F>
void cosmic::change_solvable_significant(C& cosm, F&& callback) {
	auto status = changer_callback_result::INVALID;

	auto refresh_when_done = augs::scope_guard([&]() {
		if (status != changer_callback_result::DONT_REFRESH) {
			reinfer_solvable(cosm);
		}
	});

	status = callback(cosm.get_solvable({}).significant);
}
