#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"

#include "augs/math/vec2.h"
#include "augs/drawing/sprite.h"

struct editor_sprite_resource {
	// GEN INTROSPECTOR struct editor_sprite_resource
	editor_pathed_resource external_file;

	rgba default_color = white;
	bool tile_excess_size = false;
	// END GEN INTROSPECTOR

	editor_sprite_resource(const editor_pathed_resource& f) : external_file(f) {}
};
