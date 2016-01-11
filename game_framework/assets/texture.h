#pragma once

//#include "asset.h"
//
//namespace augs {
//	class texture;
//}

namespace assets {
	enum texture_id {
		BLANK,

		CRATE,

		TEST_CROSSHAIR,
		TEST_PLAYER,

		TORSO_MOVING_FIRST,
		TORSO_MOVING_LAST = TORSO_MOVING_FIRST + 5,

		TEST_BACKGROUND,

		CUSTOM = 10000
	};
	
	//typedef asset<texture_ids, augs::texture> texture_id;
}