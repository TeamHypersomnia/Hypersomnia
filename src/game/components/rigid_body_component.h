#pragma once
#include <optional>

#include "3rdparty/Box2D/Common/b2Math.h"

#include "augs/math/transform.h"
#include "augs/pad_bytes.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/templates/maybe_const.h"

#include "game/cosmos/entity_type_traits.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

#include "augs/math/si_scaling.h"
#include "game/enums/rigid_body_type.h"

#include "augs/math/physics_structs.h"
#include "game/detail/physics/colliders_connection.h"

using rgba_channel = unsigned char;

struct friction_connection {
	// GEN INTROSPECTOR struct friction_connection
	signi_entity_id target;
	unsigned fixtures_connected = 0;
	// END GEN INTROSPECTOR
};

using friction_connection_vector = 
	augs::constant_size_vector<friction_connection, OWNER_FRICTION_GROUNDS_COUNT>
;

struct special_physics {
	// GEN INTROSPECTOR struct special_physics
	augs::stepped_cooldown dropped_or_created_cooldown;
	signi_entity_id during_cooldown_ignore_collision_with;
	bool during_cooldown_ignore_other_cooled_down = true;
	rgba_channel teleport_decrease_opacity_to = 0;
	pad_bytes<2> pad;
	real32 teleport_progress = 0.0f;
	real32 teleport_progress_falloff_speed = 0.0f;
	signi_entity_id inside_portal;
	vec2 saved_velocity;
	real32 saved_angular_velocity = 0.f;
	real32 penetrability = 1.0f;
#if TODO_CARS
	signi_entity_id owner_friction_ground;
	friction_connection_vector owner_friction_grounds = {};
#endif
	// END GEN INTROSPECTOR
	//float measured_carried_mass = 0.f;

	float get_teleport_alpha() const;
};

struct physics_engine_transforms {
	// GEN INTROSPECTOR struct physics_engine_transforms
	b2Transform m_xf;
	b2Sweep m_sweep;
	// END GEN INTROSPECTOR

	void set(const transformr&);
	void set(
		const si_scaling,
		const transformr&
	);

	transformr get(const si_scaling) const;
	transformr get() const;
};

namespace components {
	struct rigid_body {
		static constexpr bool is_synchronized = true;

		rigid_body(
			const si_scaling = si_scaling(),
			const transformr t = transformr()
		);

		// GEN INTROSPECTOR struct components::rigid_body
		physics_engine_transforms physics_transforms;

		vec2 velocity;
		real32 angular_velocity = 0.f;

		special_physics special;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct rigid_body {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::rigid_body
		rigid_body_type body_type = rigid_body_type::DYNAMIC;

		bool bullet = false;
		bool angled_damping = false;
		bool allow_sleep = true;
		bool fixed_rotation = false;

		damping_mults damping;
		// END GEN INTROSPECTOR
	};
}
