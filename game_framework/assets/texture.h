#pragma once

//#include "asset.h"
//
//namespace augs {
//	class texture;
//}

#include "math/vec2.h"

namespace assets {
	enum texture_id {
		BLANK,

		CRATE,
		CAR_INSIDE,
		CAR_FRONT,

		TRUCK_INSIDE,
		TRUCK_FRONT,

		MOTORCYCLE_FRONT,
		MOTORCYCLE_INSIDE,

		TEST_CROSSHAIR,
		TEST_PLAYER,

		TORSO_MOVING_FIRST,
		TORSO_MOVING_LAST = TORSO_MOVING_FIRST + 5,

		TEST_BACKGROUND,
		TEST_SPRITE,
		MOTOR,

		CUSTOM = 10000
	};
	
	vec2i get_size(texture_id);
	//typedef asset<texture_ids, augs::texture> texture_id;
}