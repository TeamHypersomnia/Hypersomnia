#pragma once
#include "augs/misc/enum/enum_map.h"
#include "augs/audio/sound_buffer.h"

#include "view/viewables/particle_types.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/sound_buffer_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/assets/ids/physical_material_id.h"
#include "game/assets/ids/recoil_player_id.h"

#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

#include "view/viewables/regeneration/image_loadables_def.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/image_meta.h"
#include "game/assets/ids/asset_ids.h"

#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;

struct all_viewables_defs {
	static const all_viewables_defs empty;

	// GEN INTROSPECTOR struct all_viewables_defs
	sound_buffer_inputs_map sounds;
	particle_effects_map particle_effects;
	image_loadables_map image_loadables;
	image_metas_map image_metas;
	// END GEN INTROSPECTOR

	void clear();
};

template <class I, class T>
decltype(auto) get_pool_for(T&& t) {
	if constexpr(std::is_same_v<I, assets::image_id>) {
		return t.image_loadables;
	}
	else if constexpr(std::is_same_v<I, assets::sound_buffer_id>) {
		return t.sounds;
	}
	else if constexpr(std::is_same_v<I, assets::particle_effect_id>) {
		return t.particle_effects;
	}
	else {
		static_assert(always_false_v<I>, "Unknown id type.");
	}
}

std::optional<assets::image_id> find_asset_id_by_path(
	const maybe_official_path& p,
	const image_loadables_map&
);
