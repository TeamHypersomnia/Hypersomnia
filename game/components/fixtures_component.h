#pragma once
#include <vector>
#include <array>

#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "transform_component.h"
#include "game/transcendental/component_synchronizer.h"
#include "game/enums/colliders_offset_type.h"
#include "game/detail/convex_partitioned_shape.h"
#include <Box2D/Dynamics/b2Fixture.h>

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

class physics_system;
struct colliders_cache;

namespace components {
	struct fixtures : synchronizable_component {
		struct convex_partitioned_collider {
			convex_partitioned_shape shape;
			b2Filter filter;

			float density = 1.f;
			float density_multiplier = 1.f;
			float friction = 0.f;
			float restitution = 0.f;

			bool sensor = false;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(shape),
					CEREAL_NVP(filter),

					CEREAL_NVP(density),
					CEREAL_NVP(density_multiplier),
					CEREAL_NVP(friction),
					CEREAL_NVP(restitution),

					CEREAL_NVP(sensor)
				);
			}
		};

		augs::constant_size_vector<convex_partitioned_collider, COLLIDERS_COUNT> colliders;
		std::array<components::transform, colliders_offset_type::OFFSET_COUNT> offsets_for_created_shapes;

		bool activated = true;
		bool is_friction_ground = false;
		bool disable_standard_collision_resolution = false;
		bool can_driver_shoot_through = false;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(colliders),
				CEREAL_NVP(activated),

				CEREAL_NVP(offsets_for_created_shapes),

				CEREAL_NVP(is_friction_ground),
				CEREAL_NVP(disable_standard_collision_resolution)
			);
		}

		convex_partitioned_collider& new_collider() {
			colliders.push_back(convex_partitioned_collider());
			return *(colliders.end()-1);
		}
	};
}

template<bool is_const>
class basic_fixtures_synchronizer : public component_synchronizer_base<is_const, components::fixtures> {
protected:
	friend class ::physics_system;

	maybe_const_ref_t<is_const, colliders_cache>& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::fixtures>::component_synchronizer_base;

	components::transform get_offset(colliders_offset_type) const;
	components::transform get_total_offset() const;

	bool is_activated() const;
	bool is_constructed() const;

	float get_density_multiplier(size_t = 0) const;
	float get_friction(size_t = 0) const;
	float get_restitution(size_t = 0) const;
	float get_density(size_t = 0) const;
	
	basic_entity_handle<is_const> get_owner_body() const;

	vec2 get_aabb_size() const;
	augs::rects::ltrb<float> get_aabb_rect() const;

	size_t get_num_colliders() const;

	bool is_friction_ground() const;
	bool standard_collision_resolution_disabled() const;
	bool can_driver_shoot_through() const;
};

template<>
class component_synchronizer<false, components::fixtures> : public basic_fixtures_synchronizer<false> {
	void rebuild_density(size_t) const;

	void resubstantiation() const;

public:
	using basic_fixtures_synchronizer<false>::basic_fixtures_synchronizer;

	void set_density(float, size_t = 0) const;
	void set_density_multiplier(float, size_t = 0) const;
	void set_friction(float, size_t = 0) const;
	void set_restitution(float, size_t = 0) const;
	void set_owner_body(entity_id) const;
	component_synchronizer& operator=(const components::fixtures&);
	void set_offset(colliders_offset_type, components::transform) const;
	void set_activated(bool) const;
};

template<>
class component_synchronizer<true, components::fixtures> : public basic_fixtures_synchronizer<true> {
public:
	using basic_fixtures_synchronizer<true>::basic_fixtures_synchronizer;
};