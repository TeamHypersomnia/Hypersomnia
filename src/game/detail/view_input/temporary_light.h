#pragma once
#include "game/components/light_component.h"

struct temporary_light {
	absolute_or_local positioning;
	rgba color;
	real32 radius = 0.0f;
	real32 max_lifetime_ms = 0.0f;
	real32 current_lifetime_ms = 0.0f;
	bool cast_shadow = true;

	bool is_dead() const {
		return current_lifetime_ms >= max_lifetime_ms;
	}

	real32 get_attenuation_mult() const;
	components::light to_light_component() const;
};

