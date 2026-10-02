#pragma once
#include "augs/pad_bytes.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/value_meter.h"

#include "game/assets/ids/asset_ids.h"
#include "game/components/transform_component.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/enums/adverse_element_type.h"
#include "game/detail/sentience_shake.h"
#include "game/detail/damage/damage_definition.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/predictability_info.h"
#include "game/detail/view_input/explosion_particles_def.h"

struct damage_cause;

/*
	leaves_ground_decal leaves a scorch mark on the ground, sized by the damage -
	or ground_decal_fixed_size_mult if above 0, e.g. for flashes, which deal negligible damage.
	leaves_surface_decals leaves marks on the surfaces the blast hits whose materials define explosion_decals.
	draws_color_rings draws the flat colored rings of the explosion.
	thunders_mult scales the number of branches of the thunders (with create_thunders_effect).
*/

struct standard_explosion_input {
	// GEN INTROSPECTOR struct standard_explosion_input
	real32 effective_radius = 250.f;

	damage_definition damage;

	sentience_shake subject_shake;

	real32 subject_impulse = 0.f;
	real32 subject_inert_ms = 0.f;

	real32 wave_shake_radius_mult = 2.f;
	rgba inner_ring_color = cyan;
	rgba outer_ring_color = white;
	sound_effect_input sound;
	real32 ring_duration_seconds = 0.20f;
	adverse_element_type type = adverse_element_type::FORCE;
	bool create_thunders_effect = false;
	bool hit_friendlies = true;
	bool bother_avoiding = true;
	bool leaves_ground_decal = true;
	bool leaves_surface_decals = true;
	bool draws_color_rings = true;
	pad_bytes<2> pad;
	real32 ground_decal_fixed_size_mult = 0.f;
	real32 thunders_mult = 1.f;
	explosion_particles_def explosion_particles;
	// END GEN INTROSPECTOR

	auto& operator*=(const real32 scalar) {
		damage *= scalar;
		effective_radius *= scalar;
		subject_shake *= scalar;
		subject_impulse *= scalar;
		subject_inert_ms *= scalar;

		return *this;
	}

	void instantiate(
		logic_step step, 
		transformr explosion_location, 
		damage_cause cause,
		predictability_info info = always_predictable_v
	) const;
};
