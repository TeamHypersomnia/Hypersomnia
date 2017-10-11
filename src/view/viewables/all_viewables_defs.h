#pragma once
#include "augs/misc/enum/enum_associative_array.h"
#include "augs/audio/sound_buffer.h"

#include "game/build_settings.h"
#include "view/viewables/particle_types.h"

#include "game/assets/ids/game_image_id.h"
#include "game/assets/ids/animation_id.h"
#include "game/assets/ids/sound_buffer_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/assets/ids/physical_material_id.h"
#include "game/assets/ids/recoil_player_id.h"

#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

#include "view/viewables/regeneration/game_image_loadables.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/game_image.h"

#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;

struct all_viewables_defs {
	static const all_viewables_defs empty;

	// GEN INTROSPECTOR struct all_viewables_defs
	sound_buffer_inputs_map sounds;
	particle_effects_map particle_effects;
	game_image_loadables_map game_image_loadables;
	game_image_metas_map game_image_metas;
	// END GEN INTROSPECTOR

	void update_into(all_logical_assets&);
};