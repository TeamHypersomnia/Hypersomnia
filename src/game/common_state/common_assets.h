#pragma once
#include <unordered_map>

#include "game/assets/ids/asset_ids.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

#include "game/detail/footstep_effect.h"
#include "game/cosmos/entity_flavour_id.h"
#include "augs/misc/constant_size_vector.h"
#include "game/components/touch_collectible.h"

using material_decal_variants = augs::constant_size_vector<typed_entity_flavour_id<decal_decoration>, 4>;

/*
	Decals spawned on surfaces of the given physical material.
	This lives here and not in physical_material itself,
	because assets must not hold flavour ids.
*/
struct material_decals_def {
	// GEN INTROSPECTOR struct material_decals_def
	material_decal_variants gunshot_decals;
	material_decal_variants melee_decals;
	material_decal_variants explosion_decals;
	// END GEN INTROSPECTOR
};

struct common_assets {
	// GEN INTROSPECTOR struct common_assets
	sound_effect_input ped_shield_impact_sound;
	sound_effect_input ped_shield_destruction_sound;
	sound_effect_input cast_unsuccessful_sound;

	sound_effect_input item_throw_sound;
	sound_effect_input item_holster_sound;
	sound_effect_input item_pickup_to_deposit_sound;

	particle_effect_input item_pickup_particles;
	particle_effect_input exhausted_smoke_particles;

	assets::particle_effect_id exploding_ring_smoke;
	assets::particle_effect_id exploding_ring_sparkles;
	assets::particle_effect_id thunder_remnants;

	footstep_effect_input standard_footstep;
	sound_effect_input blood_footstep_sound;
	particle_effect_input haste_footstep_particles;

	sound_effect_input standard_learnt_spell_sound;
	particle_effect_input standard_learnt_spell_particles;

	sound_effect_input flash_noise_sound;

	assets::image_id broken_shield_icon;

	per_actual_faction<assets::image_id> head_icons;
	per_actual_faction<assets::image_id> broken_head_icons;

	typed_entity_flavour_id<decal_decoration> blood_splatter_1;
	typed_entity_flavour_id<decal_decoration> blood_splatter_2;
	typed_entity_flavour_id<decal_decoration> blood_splatter_3;
	typed_entity_flavour_id<decal_decoration> blood_footstep_1;
	typed_entity_flavour_id<decal_decoration> blood_footstep_2;
	typed_entity_flavour_id<decal_decoration> blood_footstep_1_weak;
	typed_entity_flavour_id<decal_decoration> blood_footstep_2_weak;
	typed_entity_flavour_id<decal_decoration> blood_footstep_3_weak;

	typed_entity_flavour_id<decal_decoration> explosion_decal_1;
	typed_entity_flavour_id<decal_decoration> explosion_decal_2;

	std::unordered_map<assets::physical_material_id, material_decals_def> material_decals;

	particle_effect_input blood_burst_particles;

	footstep_effect_input lying_corpse_footstep;

	augs::constant_size_vector<constrained_entity_flavour_id<invariants::touch_collectible>, 4> default_coin_flavours;
	// END GEN INTROSPECTOR
};