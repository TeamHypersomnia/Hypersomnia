#pragma once
#include "game/transcendental/entity_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/transcendental/entity_flavour_id.h"

namespace components {
	struct catridge {
		// GEN INTROSPECTOR struct components::catridge
		entity_type_id shell_flavour;
		child_entity_id round;

		particle_effect_input shell_trace_particles;
		// END GEN INTROSPECTOR
	};
}