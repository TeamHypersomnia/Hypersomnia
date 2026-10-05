#pragma once
#include "game/components/light_component.h"

struct temporary_light {
	absolute_or_local positioning;
	rgba color;
	real32 radius = 0.0f;
	real32 intensity = 1.0f;
	real32 falloff = 0.5f;

	/*
		Fitted once when the flash is spawned, at its full radius - see light_falloff::calc_legacy.
		Wall lights take only the color of the light, not its alpha.
	*/

	legacy_attenuation legacy;
	legacy_attenuation wall_legacy;
	real32 max_lifetime_ms = 0.0f;
	real32 current_lifetime_ms = 0.0f;
	bool cast_shadow = true;

	bool is_dead() const {
		return current_lifetime_ms >= max_lifetime_ms;
	}

	real32 get_attenuation_mult() const;
	components::light to_light_component() const;
};

