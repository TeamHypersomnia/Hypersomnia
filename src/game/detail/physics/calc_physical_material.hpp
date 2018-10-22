#pragma once
#include "game/detail/explosive/like_explosive.h"

template <class E>
assets::physical_material_id calc_physical_material(const E& self) {
	if (const auto fuse = self.template find<invariants::hand_fuse>()) {
		if (is_like_thrown_explosive(self)) {
			if (fuse->released_physical_material.is_set()) {
				return fuse->released_physical_material;
			}
		}
	}

	if (const auto fixtures = self.template find<invariants::fixtures>()) {
		return fixtures->material;
	}

	return {};
}
