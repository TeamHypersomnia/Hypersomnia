#pragma once
#include "3rdparty/Box2D/Common/b2Math.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/templates/maybe_const.h"

#include "game/transcendental/component_synchronizer.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/math/si_scaling.h"
#include "game/enums/rigid_body_type.h"

#include "game/detail/physics/damping_info.h"
#include "game/components/transform_component.h"

class relational_cache;

struct friction_connection {
	// GEN INTROSPECTOR struct friction_connection
	entity_id target;
	unsigned fixtures_connected = 0;
	// END GEN INTROSPECTOR
	friction_connection(entity_id t = entity_id()) : target(t) {}

	operator entity_id() const {
		return target;
	}
};

struct special_physics {
	// GEN INTROSPECTOR struct special_physics
	augs::stepped_cooldown dropped_or_created_cooldown;
	entity_id during_cooldown_ignore_collision_with;
	entity_id owner_friction_ground;
	augs::constant_size_vector<friction_connection, OWNER_FRICTION_GROUNDS_COUNT> owner_friction_grounds;
	// END GEN INTROSPECTOR

	//float measured_carried_mass = 0.f;
};

template <bool, class>
class basic_physics_mixin;

namespace components {
	struct rigid_body {
		static constexpr bool is_synchronized = true;

		rigid_body(
			const si_scaling = si_scaling(),
			const components::transform t = components::transform()
		);

		// GEN INTROSPECTOR struct components::rigid_body
		b2Transform transform;
		b2Sweep sweep;

		vec2 velocity;
		float angular_velocity = 0.f;

		special_physics special;

		// END GEN INTROSPECTOR

		void set_transform(
			const si_scaling,
			const components::transform&
		);
	};
}

namespace definitions {
	struct rigid_body {
		using implied_component = components::rigid_body;

		// GEN INTROSPECTOR struct definitions::rigid_body
		bool bullet = false;
		bool angled_damping = false;
		bool allow_sleep = true;

		rigid_body_type body_type = rigid_body_type::DYNAMIC;

		damping_info damping;
		// END GEN INTROSPECTOR
	};
};

class physics_world_cache;
struct rigid_body_cache;

class b2Body;

template <bool is_const>
class basic_physics_synchronizer : public component_synchronizer_base<is_const, components::rigid_body> {
protected:
	friend class ::physics_world_cache;

	const rigid_body_cache& get_cache() const;
	const b2Body& body() const;

	template <class T>
	auto to_pixels(const T meters) const {
		return handle.get_cosmos().get_si().get_pixels(meters);
	}

	template <class T>
	auto to_meters(const T pixels) const {
		return handle.get_cosmos().get_si().get_meters(pixels);
	}

	using base = component_synchronizer_base<is_const, components::rigid_body>;
	using base::handle;

public:
	using base::component_synchronizer_base;
	using base::get_raw_component;

	bool is_activated() const;
	bool is_constructed() const;

	auto& get_special() const {
		return get_raw_component().special;
	}

	damping_info calculate_damping_info(const definitions::rigid_body&) const;

	vec2 get_velocity() const;
	float get_mass() const;
	float get_degrees() const;
	float get_radians() const;
	float get_degree_velocity() const;
	float get_radian_velocity() const;
	float get_inertia() const;
	vec2 get_position() const;
	components::transform get_transform() const;
	vec2 get_mass_position() const;
	vec2 get_world_center() const;
	bool test_point(const vec2) const;
};

template<>
class component_synchronizer<false, components::rigid_body> : public basic_physics_synchronizer<false> {
	rigid_body_cache& get_cache() const;

public:
	void infer_caches() const;

	using basic_physics_synchronizer<false>::basic_physics_synchronizer;

	void set_velocity(const vec2) const;
	void set_angular_velocity(const float) const;

	void set_transform(const components::transform&) const;
	void set_transform(const entity_id) const;

	void apply_force(const vec2) const;
	void apply_force(const vec2, const vec2 center_offset, const bool wake = true) const;
	void apply_impulse(const vec2) const;
	void apply_impulse(const vec2, const vec2 center_offset, const bool wake = true) const;
	void apply_angular_impulse(const float) const;
};

template<>
class component_synchronizer<true, components::rigid_body> : public basic_physics_synchronizer<true> {
public:
	using basic_physics_synchronizer<true>::basic_physics_synchronizer;
};
