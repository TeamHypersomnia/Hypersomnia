#pragma once
#include "game/transcendental/entity_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/transcendental/entity_flavour_id.h"

namespace invariants {
	struct catridge {
		// GEN INTROSPECTOR struct invariants::catridge
		entity_type_id shell_flavour;
		entity_type_id round_flavour;

		particle_effect_input shell_trace_particles;
		// END GEN INTROSPECTOR
	};
}