#pragma once
#include "game/assets/assets_declarations.h"
#include "game/hardcoded_content/requisite_collections.h"

namespace augs {
	struct baked_font;
}

struct game_gui_context_dependencies {
	const game_image_definitions& game_image_defs;
	const game_images_in_atlas& game_images;
	const requisite_images_in_atlas& requisite_images;
	const augs::baked_font& gui_font;
};