#pragma once
#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/ids/physical_material_id.h"

#include "game/components/transform_component.h"

class physics_world_cache;
struct motor_joint_cache;
struct b2Fixture_index_in_component;

namespace components {
	struct motor_joint {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::motor_joint
		std::array<entity_id, 2> target_bodies;
		
		bool activated = true;
		bool collide_connected = false;
		pad_bytes<2> pad;

		vec2 linear_offset;
		float angular_offset = 0.f;
		float max_force = 1.f;
		float max_torque = 1.f;
		float correction_factor = 0.3f;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_motor_joint_synchronizer : public component_synchronizer_base<is_const, components::motor_joint> {
protected:
	friend class ::physics_world_cache;

	using base = component_synchronizer_base<is_const, components::motor_joint>;
	using base::handle;
public:
	using base::get_raw_component;
	using base::component_synchronizer_base;

	bool is_activated() const;
	decltype(components::motor_joint::target_bodies) get_target_bodies() const;
};

template<>
class component_synchronizer<false, components::motor_joint> : public basic_motor_joint_synchronizer<false> {
	void regenerate_caches() const;

public:
	using basic_motor_joint_synchronizer<false>::basic_motor_joint_synchronizer;

	const component_synchronizer& operator=(const components::motor_joint& m) const;
};

template<>
class component_synchronizer<true, components::motor_joint> : public basic_motor_joint_synchronizer<true> {
public:
	using basic_motor_joint_synchronizer<true>::basic_motor_joint_synchronizer;
};