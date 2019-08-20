#pragma once
#include "game/cosmos/get_corresponding.h"

template <class E, class T>
void snap_interpolated_to(const E& handle, const T& transform) {
	auto& interp = get_corresponding<components::interpolation>(handle);
	interp.interpolated_transform = interp.desired_transform = transform;
}
