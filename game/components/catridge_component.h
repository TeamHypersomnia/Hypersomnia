#pragma once
#include "game/transcendental/entity_id.h"
#include "game/assets/particle_effect_id.h"

namespace components {
	struct catridge {
		// GEN INTROSPECTOR struct components::catridge
		child_entity_id shell;
		child_entity_id round;

		_particle_effect_response shell_trace_particle_effect_response;
		// END GEN INTROSPECTOR
	};
}