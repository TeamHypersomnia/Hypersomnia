#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/transcendental/component_synchronizer.h"
#include "game/components/transform_component.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "Box2D/Common/b2Math.h"

extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

template<bool> class basic_physics_synchronizer;
class physics_system;
struct rigid_body_cache;

namespace components {
	struct physics : synchronizable_component {
		enum class type {
			STATIC,
			KINEMATIC,
			DYNAMIC
		};

		physics();

		bool fixed_rotation = false;
		bool bullet = false;
		bool angled_damping = false;
		bool activated = true;

		type body_type = type::DYNAMIC;

		float angular_damping = 6.5f;
		float linear_damping = 6.5f;
		vec2 linear_damping_vec;
		float gravity_scale = 0.f;

		b2Transform transform;
		b2Sweep sweep;

		vec2 velocity;
		float angular_velocity = 0.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(transform),

				CEREAL_NVP(activated),

				CEREAL_NVP(body_type),

				CEREAL_NVP(angular_damping),
				CEREAL_NVP(linear_damping),
				CEREAL_NVP(linear_damping_vec),
				CEREAL_NVP(gravity_scale),

				CEREAL_NVP(fixed_rotation),
				CEREAL_NVP(bullet),
				CEREAL_NVP(angled_damping),

				CEREAL_NVP(velocity),
				CEREAL_NVP(angular_velocity)
			);
		}
	};
	
	struct fixtures;
}

template<bool is_const>
class basic_physics_synchronizer : public component_synchronizer_base<is_const, components::physics> {
protected:
	friend class ::physics_system;
	friend class component_synchronizer<is_const, components::fixtures>;
	template<bool> friend class basic_fixtures_synchronizer;

	maybe_const_ref_t<is_const, rigid_body_cache>& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::physics>::component_synchronizer_base;

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
	vec2 get_aabb_size() const;

	components::physics::type get_body_type() const;

	std::vector<basic_entity_handle<is_const>> get_fixture_entities() const;

	bool test_point(vec2) const;
};

template<>
class component_synchronizer<false, components::physics> : public basic_physics_synchronizer<false> {
	void resubstantiation() const;
public:
	using basic_physics_synchronizer<false>::basic_physics_synchronizer;

	void set_body_type(components::physics::type) const;
	void set_activated(bool) const;
	void set_velocity(vec2) const;
	void set_angular_velocity(float) const;
	void set_transform(components::transform) const;
	void set_transform(entity_id) const;
	void set_angular_damping(float) const;
	void set_linear_damping(float) const;
	void set_linear_damping_vec(vec2) const;

	void apply_force(vec2) const;
	void apply_force(vec2, vec2 center_offset, bool wake = true) const;
	void apply_impulse(vec2) const;
	void apply_impulse(vec2, vec2 center_offset, bool wake = true) const;
	void apply_angular_impulse(float) const;
};

template<>
class component_synchronizer<true, components::physics> : public basic_physics_synchronizer<true> {
public:
	using basic_physics_synchronizer<true>::basic_physics_synchronizer;
};
