#pragma once
#include "augs/templates/tuple_of_containers.h"
#include "augs/misc/convex_partitioned_shape.h"
#include "augs/misc/trivially_copyable_tuple.h"

#include "game/assets/all_logical_assets_declarations.h"

#include "game/assets/game_image_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/physical_material_id.h"
#include "game/assets/recoil_player_id.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

struct sound_buffer_logical {
	// GEN INTROSPECTOR struct augs::sound_buffer_logical
	float max_duration_in_seconds = 0.f;
	unsigned num_of_variations = 0xdeadbeef;
	// END GEN INTROSPECTOR
};

struct game_image_logical {
	// GEN INTROSPECTOR struct game_image_logical
	vec2u original_image_size;
	convex_partitioned_shape shape;
	// END GEN INTROSPECTOR

	vec2u get_size() const {
		return original_image_size;
	}
};

struct particle_effect_logical {
	// GEN INTROSPECTOR struct particle_effect_logical
	float max_duration_in_seconds;
	// END GEN INTROSPECTOR
};

template <class enum_key, class mapped>
using asset_map = augs::enum_associative_array<enum_key, mapped>;

using tuple_of_logical_assets = augs::trivially_copyable_tuple<
	asset_map<assets::game_image_id, game_image_logical>,
	asset_map<assets::particle_effect_id, particle_effect_logical>,
	asset_map<assets::sound_buffer_id, sound_buffer_logical>,

	asset_map<assets::animation_id, animation>,
	asset_map<assets::recoil_player_id, recoil_player>,
	asset_map<assets::physical_material_id, physical_material>
>;

struct all_logical_assets 
	: public tuple_of_containers<tuple_of_logical_assets>
{};