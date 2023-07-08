#pragma once
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"
#include "augs/math/physics_structs.h"

enum class marker_shape_type {
	// GEN INTROSPECTOR enum class marker_shape_type
	BOX,
	CIRCLE,
	COUNT
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct point_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::point_marker
		point_marker_type type = point_marker_type::TEAM_SPAWN;
		// END GEN INTROSPECTOR
	};

	struct area_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::area_marker
		area_marker_type type = area_marker_type::BOMBSITE;
		// END GEN INTROSPECTOR
	};
}

enum class force_field_falloff {
	// GEN INTROSPECTOR enum class force_field_falloff
	NONE,
	LINEAR,
	QUADRATIC,
	SQRT,
	COUNT
	// END GEN INTROSPECTOR
};

enum class force_field_direction {
	// GEN INTROSPECTOR enum class force_field_direction
	FIELD_DIRECTION,
	INWARD,
	OUTWARD,
	CIRCULAR,
	COUNT
	// END GEN INTROSPECTOR
};

struct hazard_def {
	// GEN INTROSPECTOR struct hazard_def
	float damage = 35.0f;
	// END GEN INTROSPECTOR

	bool operator==(const hazard_def&) const = default;
};

struct force_field_def {
	// GEN INTROSPECTOR struct force_field_def
	force_field_direction direction = force_field_direction::INWARD;
	force_field_falloff falloff = force_field_falloff::QUADRATIC;

	float character_force = 3000.0f;
	float character_target_airborne_ms = 300.0f;
	float character_airborne_acceleration = 0.2f;

	float object_force = 300.0f;
	float object_torque = 10.0f;

	bool mass_invariant = true;
	bool stronger_towards_center = false;
	bool fly_even_without_sprint = false;
	bool falloff_decreases_airborne_acceleration = true;
	// END GEN INTROSPECTOR

	bool operator==(const force_field_def&) const = default;
};

struct portal_exit_impulses {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct portal_exit_impulses
	float character_exit_airborne_ms = 400.0f;
	impulse_amount_def character_exit_impulse = { 10000.0f, impulse_type::ADD_VELOCITY };

	impulse_amount_def object_exit_impulse = { 6000.0f, impulse_type::ADD_VELOCITY };
	impulse_amount_def object_exit_angular_impulse = { 1000.0f, impulse_type::ADD_VELOCITY };
	// END GEN INTROSPECTOR

	bool operator==(const portal_exit_impulses&) const = default;

	void set_zero() {
		character_exit_airborne_ms = 0.0f;
		character_exit_impulse.set_zero();
		object_exit_impulse.set_zero();
		object_exit_angular_impulse.set_zero();
	}
};

namespace components {
	struct marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct components::marker
		faction_type faction = faction_type::METROPOLIS;
		marker_letter_type letter = marker_letter_type::A;
		marker_shape_type shape = marker_shape_type::BOX;
		// END GEN INTROSPECTOR

	};
}
