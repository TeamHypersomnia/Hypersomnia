#include "crosshair_component.h"
#include "game/transcendental/entity_id.h"
#include "transform_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

namespace components {
	vec2 crosshair::calculate_aiming_displacement(
		const const_entity_handle subject_crosshair, 
		const bool snap_epsilon_base_offset
	) {
		const auto recoil_body = subject_crosshair[sub_entity_name::CROSSHAIR_RECOIL_BODY];
		const auto recoil_body_transform = recoil_body.get_logic_transform();

		auto considered_base_offset = subject_crosshair.get<components::crosshair>().base_offset;

		if (snap_epsilon_base_offset && considered_base_offset.is_epsilon(4)) {
			considered_base_offset.set(4, 0);
		}

		considered_base_offset += recoil_body_transform.pos;

		considered_base_offset.rotate(recoil_body_transform.rotation, vec2());

		return considered_base_offset;
	}
}