#pragma once
#include "game/cosmos/get_corresponding.h"

template <class E, class T>
void snap_interpolated_to(const E& handle, const T& transform) {
	get_corresponding<components::interpolation>(handle).snap_to(transform);
}
