#pragma once
#include "game/transcendental/entity_handle.h"

struct viewer_eye {
	const_entity_handle viewed_character;
	camera_eye cone;
	vec2 screen_size;
};
