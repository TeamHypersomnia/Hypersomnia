#pragma once
#include "game/enums/slot_function.h"

template <class E>
bool wears_backpack(const E& character) {
	const auto back_slot = character[slot_function::BACK];
	return back_slot.alive() && back_slot.has_items();
}
