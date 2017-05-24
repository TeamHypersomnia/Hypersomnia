#pragma once
#include "game/transcendental/entity_id.h"
#include "game/assets/particle_effect_id.h"

namespace components {
	struct catridge {
		// GEN INTROSPECTOR struct components::catridge
		child_entity_id shell;
		child_entity_id round;

		particle_effect_response shell_trace_particles;
		// END GEN INTROSPECTOR
	};
}