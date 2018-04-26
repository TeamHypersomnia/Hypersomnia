#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/asset_containers.h"

#include "view/viewables/loaded_sounds_map.h"

namespace augs {
	class sound_buffer;
	struct sound_buffer_loading_input;
}

struct image_in_atlas;
struct image_definition;
class image_definition_view;
struct image_meta;
struct image_cache;
struct particle_effect;

using particle_effects_map = asset_map<
	assets::particle_effect_id,
	particle_effect
>;

using image_definitions_map = image_id_pool<image_definition>;
using sound_buffer_inputs_map = sound_id_pool<augs::sound_buffer_loading_input>;

class images_in_atlas_map;
