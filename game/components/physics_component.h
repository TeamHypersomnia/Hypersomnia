#pragma once
#include "misc/stepped_timing.h"
#include "game/component_synchronizer.h"
#include "game/components/transform_component.h"
#include "game/entity_handle_declaration.h"

extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

namespace components {
	struct physics {
	private:
		std::vector<entity_id> fixture_entities;
		friend class component_synchronizer<false, components::fixtures>;
	public:
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
struct rigid_body_cache;

template<bool is_const>
class component_synchronizer<is_const, components::physics> : public component_synchronizer_base<is_const, components::physics> {
	friend class ::physics_system;
	friend class component_synchronizer<is_const, components::fixtures>;

	typename maybe_const_ref<is_const, rigid_body_cache>::type& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::physics>::component_synchronizer_base;

	template <class = typename std::enable_if<!is_const>::type>
	void set_body_type(components::physics::type);

	template <class = typename std::enable_if<!is_const>::type>
	void set_activated(bool);

	bool is_activated() const;
	bool is_constructed() const;

	template <class = typename std::enable_if<!is_const>::type>
	void set_velocity(vec2);
	template <class = typename std::enable_if<!is_const>::type>
	void set_transform(components::transform);
	template <class = typename std::enable_if<!is_const>::type>
	void set_transform(const_entity_handle);

	template <class = typename std::enable_if<!is_const>::type>
	void set_angular_damping(float);
	template <class = typename std::enable_if<!is_const>::type>
	void set_linear_damping(float);
	template <class = typename std::enable_if<!is_const>::type>
	void set_linear_damping_vec(vec2);

	template <class = typename std::enable_if<!is_const>::type>
	void apply_force(vec2);
	template <class = typename std::enable_if<!is_const>::type>
	void apply_force(vec2, vec2 center_offset, bool wake = true);
	template <class = typename std::enable_if<!is_const>::type>
	void apply_impulse(vec2);
	template <class = typename std::enable_if<!is_const>::type>
	void apply_impulse(vec2, vec2 center_offset, bool wake = true);
	template <class = typename std::enable_if<!is_const>::type>
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

	components::physics::type get_body_type() const;

	basic_entity_handle<is_const> get_owner_friction_ground() const;

	const std::vector<basic_entity_handle<is_const>>& get_fixture_entities() const;

	bool test_point(vec2) const;
};