#pragma once
#include "game/detail/transform_copying.h"
#include "game/detail/calc_trace_scaling.h"

/*
	The offset to compose with the chased target's transform.

	With chase_sprite_back set, anchors at the rear tip of the target's rendered sprite -
	which travels backwards in the target's local space as the trace stretches the sprite
	in flight - so that e.g. trail streams always begin right behind the sprite,
	never getting covered by (or poking over) its lengthening body.

	steps_alpha is how far between the two newest steps the target is drawn,
	1.0 when chasing its logic transform.
*/
template <class S, class H>
transformr considered_chase_offset(const S& self, const H& target_handle, const float steps_alpha) {
	if (!self.chase_sprite_back) {
		return self.offset;
	}

	auto result = self.offset;

	const auto w = target_handle.get_logical_size().x;
	auto back_x = -(w / 2);

	target_handle.template dispatch_on_having_all<invariants::trace>([&](const auto& typed_target) {
		if (const auto trace = ::calc_trace_scaling(typed_target, steps_alpha)) {
			if (trace->size_mult.x > 0.f) {
				/*
					Mirrors how draw_entity.h renders traces: the sprite's width is scaled
					by size_mult and its center is shifted back by the stretched width
					times center_offset_mult.
				*/
				const auto stretched_w = w * trace->size_mult.x;
				back_x = -(stretched_w * (0.5f + trace->center_offset_mult.x));
			}
		}
	});

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

				return *target_transform * ::considered_chase_offset(self, target_handle, interp.get_bullets_alpha());
			}
		}

		return std::nullopt;
	}

	return self.offset;
}

