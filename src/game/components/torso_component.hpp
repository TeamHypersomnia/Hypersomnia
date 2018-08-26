#pragma once
#include "game/components/torso_component.h"

struct leg_animation_usage {
	assets::legs_animation_id id;
	bool flip_vertically = false;
	float rotation = 0.f;
};

namespace invariants {
	inline auto torso::calc_leg_anim(
		vec2 legs_dir,
		const real32 face_degrees
	) const {
		leg_animation_usage output;

		legs_dir.normalize();

		const auto face_dir = vec2::from_degrees(face_degrees);
		const auto facing = legs_dir.degrees_between(face_dir);

		if (facing >= min_strafe_facing && facing <= max_strafe_facing) {
			output.id = strafe_legs;

			vec2 perp_face;

			if (face_dir.cross(legs_dir) < 0.f) {
				output.flip_vertically = true;
				perp_face = face_dir.perpendicular_ccw();
			}
			else {
				perp_face = face_dir.perpendicular_cw();
			}

			const auto lerped = legs_dir.lerp(perp_face, strafe_face_interp_mult);;
			legs_dir = lerped;
		}
		else {
			if (facing > 90) {
				legs_dir.neg();
			}

			const auto lerped = legs_dir.lerp(face_dir, strafe_face_interp_mult);;
			legs_dir = lerped;

			output.id = forward_legs;
		}

		output.rotation = legs_dir.degrees();
		return output;
	}
}
