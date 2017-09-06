#pragma once
#include "augs/templates/tuple_of_containers.h"
#include "augs/misc/enum_associative_array.h"
#include "augs/audio/sound_buffer.h"

#include "game/build_settings.h"
#include "view/viewables/particle_types.h"

#include "game/assets/game_image_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/physical_material_id.h"
#include "game/assets/recoil_player_id.h"

#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

#include "view/viewables/regeneration/game_image_loadables.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/game_image.h"

#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;

struct all_viewables {
	sound_buffer_inputs_map sounds;
	particle_effects_map particle_effects;
	game_image_loadables_map game_image_loadables;
	game_image_metas_map game_image_metas;

	void update_into(all_logical_assets&);
};