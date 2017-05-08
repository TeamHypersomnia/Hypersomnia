#pragma once
#include "augs/padding_byte.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/physical_material_id.h"

#include "game/components/transform_component.h"

class physics_system;
struct motor_joint_cache;
struct b2Fixture_index_in_component;

namespace components {
	struct motor_joint : synchronizable_component {
		// GEN INTROSPECTOR struct components::motor_joint
		bool activated = true;
		std::array<padding_byte, 3> pad;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_motor_joint_synchronizer : public component_synchronizer_base<is_const, components::motor_joint> {
protected:
	friend class ::physics_system;

	maybe_const_ref_t<is_const, motor_joint_cache>& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::motor_joint>::component_synchronizer_base;

};

template<>
class component_synchronizer<false, components::motor_joint> : public basic_motor_joint_synchronizer<false> {
	void reinference() const {
	
	}

public:
	using basic_motor_joint_synchronizer<false>::basic_motor_joint_synchronizer;

	component_synchronizer& operator=(const components::motor_joint&) {
	
	}
};

template<>
class component_synchronizer<true, components::motor_joint> : public basic_motor_joint_synchronizer<true> {
public:
	using basic_motor_joint_synchronizer<true>::basic_motor_joint_synchronizer;
};