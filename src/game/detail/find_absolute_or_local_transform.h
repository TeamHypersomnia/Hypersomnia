#pragma once
#include "game/detail/transform_copying.h"

template <class S, class C, class I>
std::optional<transformr> find_transform_impl(S& self, C& cosm, I& interp) {
	if (self.target.is_set()) {
		const auto target_handle = cosm[self.target];

		if (target_handle) {
			if (auto target_transform = target_handle.find_viewing_transform(interp)) {
				if (self.face_velocity) {
					target_transform->rotation = target_handle.get_effective_velocity().degrees();
				}

				return *target_transform * self.offset;
			}
		}

		return std::nullopt;
	}

	return self.offset;
}

