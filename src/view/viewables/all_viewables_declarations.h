#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"

#include "game/assets/ids/asset_id_declarations.h"
#include "game/assets/asset_map.h"

namespace augs {
	class sound_buffer;
	struct sound_buffer_loading_input;
}

struct game_image_in_atlas;
struct image_loadables;
struct game_image_meta;
struct game_image_cache;
struct particle_effect;

using sound_buffer_inputs_map = asset_map<
	assets::sound_buffer_id,
	augs::sound_buffer_loading_input
>;

struct loaded_sounds_map;

using particle_effects_map = asset_map<
	assets::particle_effect_id,
	particle_effect
>;

using game_image_loadables_map = asset_map<
	assets::image_id,
	image_loadables
>;

using game_image_metas_map = asset_map<
	assets::image_id,
	game_image_meta
>;

using game_images_in_atlas_map = asset_map<
	assets::image_id,
	game_image_in_atlas
>;
