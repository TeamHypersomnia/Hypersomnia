#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/entity_flavour_id.h"

namespace invariants {
	struct cartridge {
		using round_flavour_type = constrained_entity_flavour_id<
			invariants::rigid_body, 
			invariants::missile, 
			components::sender
		>;

		using shell_flavour_type = constrained_entity_flavour_id<
			invariants::rigid_body
		>;

		// GEN INTROSPECTOR struct invariants::cartridge

		shell_flavour_type shell_flavour;
		round_flavour_type round_flavour;

		particle_effect_input shell_trace_particles;
		// END GEN INTROSPECTOR
	};
}