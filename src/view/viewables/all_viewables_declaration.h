#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/asset_pools.h"

#include "view/viewables/loaded_sounds_map.h"

struct image_in_atlas;
struct image_definition;
struct image_meta;
struct image_cache;
struct particle_effect;

struct sound_definition;

using image_definitions_map = make_asset_pool<image_definition, assets::image_id_key>;
using sound_definitions_map = make_asset_pool<sound_definition, assets::sound_id_key>;
using particle_effects_map = make_asset_pool<particle_effect, assets::particle_effect_id_key>;

class images_in_atlas_map;
struct image_definition_view;
