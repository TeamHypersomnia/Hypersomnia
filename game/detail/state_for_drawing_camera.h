#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/components/transform_component.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/camera_cone.h"

struct state_for_drawing_camera {
	entity_id associated_character;
	camera_cone camera;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(associated_character),
			CEREAL_NVP(camera)
		);
	}
};