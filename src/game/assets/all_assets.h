#pragma once
#include <tuple>

#include "augs/templates/tuple_of_containers.h"
#include "augs/misc/enum_associative_array.h"

#include "game/build_settings.h"
#include "game/detail/particle_types.h"

#include "game/assets/assets_declarations.h"

#include "game/assets/game_image_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/physical_material_id.h"
#include "game/assets/recoil_player_id.h"

#include "game/assets/game_image.h"
#include "game/assets/animation.h"
#include "augs/audio/sound_buffer.h"
#include "game/assets/particle_effect.h"
#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

using tuple_of_viewable_defs = std::tuple<
	game_image_definitions,
	particle_effect_definitions,
	sound_buffer_definitions
>;

using tuple_of_logical_assets = augs::trivially_copyable_tuple<
	asset_map<assets::game_image_id, game_image_logical>,
	asset_map<assets::particle_effect_id, particle_effect_logical>,
	asset_map<assets::sound_buffer_id, augs::sound_buffer_logical>,

	asset_map<assets::animation_id, animation>,
	asset_map<assets::recoil_player_id, recoil_player>,
	asset_map<assets::physical_material_id, physical_material>
>;

class all_viewable_defs : public tuple_of_containers<tuple_of_viewable_defs> {
	friend class all_logical_assets;
};

class all_logical_assets : public tuple_of_containers<tuple_of_logical_assets> {
public:
	void update_from(const all_viewable_defs&);
};

enum class viewables_loading_type {
	ALWAYS_HAVE_ALL_LOADED,
	STREAM_ONLY_NEAR_CAMERA
};