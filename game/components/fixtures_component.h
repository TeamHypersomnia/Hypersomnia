#pragma once
#include <vector>
#include <array>

#include "game/entity_id.h"
#include "math/vec2.h"
#include "math/rects.h"
#include "transform_component.h"
#include "game/component_synchronizer.h"
#include "game/enums/colliders_offset_type.h"
#include "game/detail/convex_partitioned_shape.h"
#include <Box2D/Dynamics/b2Fixture.h>

namespace components {
	struct fixtures {
	private:
		entity_id owner_body;
		friend class component_synchronizer<false, components::fixtures>;
		friend class component_synchronizer<true, components::fixtures>;
	
	public:
		struct convex_partitioned_collider {
			convex_partitioned_shape shape;
			b2Filter filter;

			float density = 1.f;
			float density_multiplier = 1.f;
			float friction = 0.f;
			float restitution = 0.f;

			bool sensor = false;
		};

		std::vector<convex_partitioned_collider> colliders;
		bool activated = true;

		std::array<components::transform, colliders_offset_type::OFFSET_COUNT> offsets_for_created_shapes;

		convex_partitioned_collider& new_collider() {
			colliders.push_back(convex_partitioned_collider());
			return *colliders.rbegin();
		}

		bool is_friction_ground = false;
		bool disable_standard_collision_resolution = false;
	};
}

struct colliders_cache;
class physics_system;

template<bool is_const>
class component_synchronizer<is_const, components::fixtures> : public component_synchronizer_base<is_const, components::fixtures> {
	template <class = std::enable_if_t<!is_const>>
	void rebuild_density(size_t);

	friend struct components::physics;
	friend class ::physics_system;

	maybe_const_ref_t<is_const, colliders_cache>& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::fixtures>::component_synchronizer_base;

	template <class = std::enable_if_t<!is_const>>
	component_synchronizer& operator=(const components::fixtures&);

	template <class = std::enable_if_t<!is_const>>
	void set_offset(colliders_offset_type, components::transform);

	components::transform get_offset(colliders_offset_type) const;
	components::transform get_total_offset() const;

	template <class = std::enable_if_t<!is_const>>
	void set_activated(bool);

	bool is_activated() const;
	bool is_constructed() const;

	template <class = std::enable_if_t<!is_const>>
	void set_density(float, size_t = 0);

	template <class = std::enable_if_t<!is_const>>
	void set_density_multiplier(float, size_t = 0);

	template <class = std::enable_if_t<!is_const>>
	void set_friction(float, size_t = 0);

	template <class = std::enable_if_t<!is_const>>
	void set_restitution(float, size_t = 0);

	float get_density_multiplier(size_t = 0) const;
	float get_friction(size_t = 0) const;
	float get_restitution(size_t = 0) const;
	float get_density(size_t = 0) const;

	template <class = std::enable_if_t<!is_const>>
	void set_owner_body(entity_id);
	
	basic_entity_handle<is_const> get_owner_body() const;

	vec2 get_aabb_size() const;
	augs::rects::ltrb<float> get_aabb_rect() const;

	size_t get_num_colliders() const;

	bool is_friction_ground() const;
	bool standard_collision_resolution_disabled() const;
};