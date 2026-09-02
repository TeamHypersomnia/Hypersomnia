#pragma once
#include "game/detail/transform_copying.h"
#include "game/components/trace_component.h"

/*
	The offset to compose with the chased target's transform.

	With chase_sprite_back set, anchors at the rear tip of the target's rendered sprite -
	which travels backwards in the target's local space as the trace stretches the sprite
	in flight - so that e.g. trail streams always begin right behind the sprite,
	never getting covered by (or poking over) its lengthening body.
*/
template <class S, class H>
transformr considered_chase_offset(const S& self, const H& target_handle) {
	if (!self.chase_sprite_back) {
		return self.offset;
	}

	auto result = self.offset;

	const auto w = target_handle.get_logical_size().x;
	auto back_x = -(w / 2);

	if (const auto* const trace = target_handle.template find<components::trace>()) {
		if (trace->enabled && trace->last_size_mult.x > 0.f) {
			/*
				Mirrors how draw_entity.h renders traces: the sprite's width is scaled
				by last_size_mult and its center is shifted back by the stretched width
				times last_center_offset_mult.
			*/
			const auto stretched_w = w * trace->last_size_mult.x;
			back_x = -(stretched_w * (0.5f + trace->last_center_offset_mult.x));
		}
	}

	result.pos.x += back_x;
	return result;
}

template <class S, class C, class I>
std::optional<transformr> find_transform_impl(S& self, C& cosm, I& interp) {
	if (self.target.is_set()) {
		const auto target_handle = cosm[self.target];

		if (target_handle) {
			if (auto target_transform = target_handle.find_viewing_transform(interp)) {
				if (self.face_velocity) {
					target_transform->rotation = target_handle.get_effective_velocity().degrees();
				}

				return *target_transform * ::considered_chase_offset(self, target_handle);
			}
		}

		return std::nullopt;
	}

	return self.offset;
}

