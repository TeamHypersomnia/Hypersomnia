#pragma once
#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/ids/asset_ids.h"

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

template <class E>
class component_synchronizer<E, components::motor_joint> : public synchronizer_base<E, components::motor_joint> {
protected:
	friend class ::physics_world_cache;

	using base = synchronizer_base<E, components::motor_joint>;
	using base::handle;
public:
	void infer_caches() const {
		handle.get_cosmos().get_solvable_inferred({}).relational.infer_cache_for(handle);
		handle.get_cosmos().get_solvable_inferred({}).physics.infer_cache_for(handle);
	}

	using base::get_raw_component;
	using base::base;

	bool is_activated() const{
		return get_raw_component().activated;
	}

	auto get_target_bodies() const {
		return get_raw_component().target_bodies;
	}

	auto& operator=(const components::motor_joint& m) const {
		get_raw_component({}) = m;
		infer_caches();
		return *this;
	}
};

