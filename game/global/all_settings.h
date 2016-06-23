#pragma once
#include "input_context.h"
#include "visibility_settings.h"
#include "pathfinding_settings.h"

struct all_settings {
	vec2i screen_size;
	input_context input;
	visibility_settings visibility;
	pathfinding_settings pathfinding;
};