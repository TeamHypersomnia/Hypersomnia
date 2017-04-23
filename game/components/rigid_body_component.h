#pragma once
#include "3rdparty/Box2D/Common/b2Math.h"

#include "augs/misc/stepped_timing.h"

#include "game/transcendental/component_synchronizer.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/math/si_scaling.h"
#include "game/enums/rigid_body_type.h"

#include "game/components/transform_component.h"

class physics_system;
struct rigid_body_cache;

namespace components {
	struct rigid_body : synchronizable_component {
		rigid_body(
			const si_scaling = si_scaling(),
			const components::transform t = components::transform()
		);

		// GEN INTROSPECTOR struct components::rigid_body
		bool fixed_rotation = false;
		bool bullet = false;
		bool angled_damping = false;
		bool activated = true;

		rigid_body_type body_type = rigid_body_type::DYNAMIC;

		float angular_damping = 6.5f;
		float linear_damping = 6.5f;
		vec2 linear_damping_vec;
		float gravity_scale = 0.f;

		b2Transform transform;
		b2Sweep sweep;

		vec2 velocity;
		float angular_velocity = 0.f;
		// END GEN INTROSPECTOR

		void set_transform(
			const si_scaling,
			const components::transform&
		);
	};
	
	struct fixtures;
}

template<bool is_const>
class basic_physics_synchronizer : public component_synchronizer_base<is_const, components::rigid_body> {
protected:
	friend class ::physics_system;
	friend class component_synchronizer<is_const, components::fixtures>;
	template<bool> friend class basic_fixtures_synchronizer;

	maybe_const_ref_t<is_const, rigid_body_cache>& get_cache() const;

	template <class T>
	auto to_pixels(const T meters) const {
		return handle.get_cosmos().get_si().get_pixels(meters);
	}

	template <class T>
	auto to_meters(const T pixels) const {
		return handle.get_cosmos().get_si().get_meters(pixels);
	}

public:
	using component_synchronizer_base<is_const, components::rigid_body>::component_synchronizer_base;

	bool is_activated() const;
	bool is_constructed() const;

	vec2 velocity() const;
	float get_mass() const;
	float get_angle() const;
	float get_angular_velocity() const;
	float get_inertia() const;
	vec2 get_position() const;
	vec2 get_mass_position() const;
	vec2 get_world_center() const;

	rigid_body_type get_body_type() const;

	auto get_fixture_entities() const {
		return handle.get_fixture_entities();
	}

	bool test_point(vec2) const;
};

template<>
class component_synchronizer<false, components::rigid_body> : public basic_physics_synchronizer<false> {
	void reinference() const;
public:
	using basic_physics_synchronizer<false>::basic_physics_synchronizer;

	void set_body_type(const rigid_body_type) const;
	void set_bullet_body(const bool flag) const;
	void set_activated(const bool) const;
	void set_velocity(const vec2) const;
	void set_angular_velocity(const float) const;
	void set_transform(const components::transform&) const;
	void set_transform(const entity_id) const;
	void set_angular_damping(const float) const;
	void set_linear_damping(const float) const;
	void set_linear_damping_vec(const vec2) const;

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
