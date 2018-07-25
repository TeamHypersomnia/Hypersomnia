#pragma once
#include "game/cosmos/entity_handle.h"

struct character_camera {
	const_entity_handle viewed_character;
	camera_cone cone;
};
