#pragma once
#include "game/cosmos/cosmos.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_traits.h"

template <class T, class F = int>
void sanitize_affected_entities(
	const editor_command_input& in,
	T& affected_entities,
	F&& id_getter = int()
) {
	auto& cosm = in.get_cosmos();

	auto dead_eraser = [&](const auto& entry) {
		if constexpr(std::is_same_v<remove_cref<F>, int>) {
			return cosm[entry].dead();
		}
		else {
			return cosm[id_getter(entry)].dead();
		}
	};

	erase_if(affected_entities, dead_eraser);
}

