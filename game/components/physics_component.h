#pragma once
#include "misc/deterministic_timing.h"
#include "game/detail/physics_engine_reflected_state.h"

extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

class physics_system;

namespace components {
	struct fixtures;
}

namespace components {
	struct physics : public ::rigid_body_white_box {
	private:
		rigid_body_black_box black;
		rigid_body_black_box_detail black_detail;

		bool syncable_black_box_exists() const;
		bool should_body_exist_now() const;
		
		entity_handle get_entity();

		void build_body();
		void destroy_body();

		friend class ::physics_system;
		friend struct ::components::fixtures;

	public:
		typedef rigid_body_black_box::type type;

		physics& operator=(const physics&);
		physics(const physics&);
		physics(const rigid_body_definition& = rigid_body_definition());
		void initialize_from_definition(const rigid_body_definition&);
		rigid_body_definition get_definition() const;

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
}
