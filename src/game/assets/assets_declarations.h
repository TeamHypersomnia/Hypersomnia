#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"
#include "game/container_sizes.h"

template <class enum_key, class mapped>
using asset_map = augs::enum_associative_array<enum_key, mapped>;

namespace augs {
	class sound_buffer;
	struct sound_buffer_loading_input;
}

namespace assets {
	enum class game_image_id;
	enum class sound_buffer_id;
	enum class particle_effect_id;
}

struct game_image_in_atlas;
struct game_image_definition;

struct particle_effect;

using game_images_in_atlas = asset_map<
	assets::game_image_id,
	game_image_in_atlas
>;

using game_image_definitions = asset_map<
	assets::game_image_id,
	game_image_definition
>;

using sound_buffer_definitions = asset_map<
	assets::sound_buffer_id,
	augs::sound_buffer_loading_input
>;

struct loaded_sounds;

using particle_effect_definitions = asset_map<
	assets::particle_effect_id,
	particle_effect
>;

template <class T, std::size_t, std::size_t>
struct basic_convex_partitioned_shape;

using convex_partitioned_shape = basic_convex_partitioned_shape<
	real32,
	CONVEX_POLYS_COUNT,
	CONVEX_POLY_VERTEX_COUNT
>;