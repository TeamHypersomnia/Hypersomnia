#pragma once
#include "augs/misc/enum/enum_map.h"
#include "augs/audio/sound_buffer.h"

#include "view/viewables/particle_types.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

#include "view/viewables/sound_definition.h"
#include "view/viewables/image_definition.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/image_meta.h"
#include "game/assets/ids/asset_ids.h"

#include "view/viewables/all_viewables_declarations.h"
#include "view/maybe_official_path.h"

struct all_logical_assets;

struct all_viewables_defs {
	static const all_viewables_defs empty;

	// GEN INTROSPECTOR struct all_viewables_defs
	sound_definitions_map sounds;
	particle_effects_map particle_effects;
	image_definitions_map image_definitions;
	// END GEN INTROSPECTOR

	void clear();
};

template <class I, class P>
std::optional<I> find_asset_id_by_path(
	const maybe_official_path<I>&,
	const P&
);
