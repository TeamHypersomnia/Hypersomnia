#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"

#include "game/assets/ids/asset_id_declarations.h"
#include "game/assets/asset_map.h"

namespace augs {
	class sound_buffer;
	struct sound_buffer_loading_input;
}

struct image_in_atlas;
struct image_loadables_def;
struct image_meta;
struct image_cache;
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

using image_loadables_map = image_id_map<image_loadables_def>;
using image_metas_map = image_id_map<image_meta>;
using game_images_in_atlas_map = image_id_map<image_in_atlas>;
