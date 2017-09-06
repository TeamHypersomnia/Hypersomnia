#pragma once
#include "view/viewables/game_image.h"
#include "view/viewables/regeneration/game_image_loadables.h"

struct game_image_definition {
	// GEN INTROSPECTOR struct game_image_definition
	game_image_loadables loadables;
	game_image_meta meta;

	void update_size();
	// END GEN INTROSPECTOR
};