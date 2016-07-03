#pragma once
#include "misc/stepped_timing.h"
#include "game/component_synchronizer.h"

extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

namespace components {
	struct physics {
		components::transform transform;

		bool activated = true;

		enum class type {
			DYNAMIC,
			STATIC,
			KINEMATIC
		};

		type body_type = type::DYNAMIC;

		float angular_damping = 6.5f;
		float linear_damping = 6.5f;
		vec2 linear_damping_vec;
		float gravity_scale = 0.f;

		bool fixed_rotation = false;
		bool bullet = false;
		bool angled_damping = false;

		vec2 velocity;
		float angular_velocity = 0.f;
	};
	
	struct fixtures;
}

class physics_system;

template<bool is_const>
class component_synchronizer<is_const, components::physics> : public component_synchronizer_base<is_const, components::physics> {
	bool syncable_black_box_exists() const;
	bool should_body_exist_now() const;

	void build_body();
	void destroy_body();

	friend class ::physics_system;
	friend struct ::components::fixtures;

public:
	using component_synchronizer_base<is_const, components::physics>::component_synchronizer_base;

	void set_body_type(type);
	void set_activated(bool);

	void set_velocity(vec2);
	void set_transform(components::transform);
	void set_transform(entity_id);

	void set_angular_damping(float);
	void set_linear_damping(float);
	void set_linear_damping_vec(vec2);

	void apply_force(vec2);
	void apply_force(vec2, vec2 center_offset, bool wake = true);
	void apply_impulse(vec2);
	void apply_impulse(vec2, vec2 center_offset, bool wake = true);
	void apply_angular_impulse(float);

	vec2 velocity() const;
	float get_mass() const;
	float get_angle() const;
	float get_angular_velocity() const;
	float get_inertia() const;
	vec2 get_position() const;
	vec2 get_mass_position() const;
	vec2 get_world_center() const;
	vec2 get_aabb_size() const;

	type get_body_type() const;

	bool is_activated() const;

	entity_id get_owner_friction_ground() const;

	const std::vector<entity_id>& get_fixture_entities() const;

	bool test_point(vec2) const;
};