#pragma once
#include <Box2D/Dynamics/b2Fixture.h>

#include "augs/padding_byte.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum_array.h"
#include "augs/misc/trivial_variant.h"

#include "game/container_sizes.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/physical_material_id.h"

#include "game/components/transform_component.h"

#include "game/detail/shape_variant.h"

class physics_system;
struct colliders_cache;
struct b2Fixture_index_in_component;

struct convex_partitioned_collider {
	// GEN INTROSPECTOR struct convex_partitioned_collider
	shape_variant shape;
	std::array<convex_poly_destruction_data, CONVEX_POLYS_COUNT> destruction;

	assets::physical_material_id material = assets::physical_material_id::METAL;

	float collision_sound_gain_mult = 1.f;

	float density = 1.f;
	float density_multiplier = 1.f;
	float friction = 0.f;
	float restitution = 0.f;

	b2Filter filter;
	bool destructible = false;
	bool sensor = false;
	// END GEN INTROSPECTOR
};

namespace components {
	struct fixtures : synchronizable_component {
		// GEN INTROSPECTOR struct components::fixtures
		augs::constant_size_vector<convex_partitioned_collider, COLLIDERS_COUNT> colliders;
		augs::enum_array<components::transform, colliders_offset_type> offsets_for_created_shapes;

		bool activated = true;
		bool is_friction_ground = false;
		bool disable_standard_collision_resolution = false;
		bool can_driver_shoot_through = false;
		// END GEN INTROSPECTOR

		convex_partitioned_collider& new_collider() {
			colliders.push_back(convex_partitioned_collider());
			return *(colliders.end()-1);
		}

		static components::transform transform_around_body(const const_entity_handle fixtures_entity, const components::transform& body_transform);
	};
}

template<bool is_const>
class basic_fixtures_synchronizer : public component_synchronizer_base<is_const, components::fixtures> {
protected:
	friend class ::physics_system;

	maybe_const_ref_t<is_const, colliders_cache>& get_cache() const;
public:
	using component_synchronizer_base<is_const, components::fixtures>::component_synchronizer_base;

	components::transform get_offset(const colliders_offset_type) const;
	components::transform get_total_offset() const;

	bool is_activated() const;
	bool is_constructed() const;

	float get_density_multiplier(const size_t collider_index = 0) const;
	float get_friction(const size_t collider_index = 0) const;
	float get_restitution(const size_t collider_index = 0) const;
	float get_base_density(const size_t collider_index = 0) const;
	float get_density(const size_t collider_index = 0) const;
	
	basic_entity_handle<is_const> get_owner_body() const;

	ltrb get_local_aabb() const;

	const convex_partitioned_collider& get_collider_data(const size_t collider_index) const;
	size_t get_num_colliders() const;

	bool is_friction_ground() const;
	bool standard_collision_resolution_disabled() const;
	bool can_driver_shoot_through() const;
};

template<>
class component_synchronizer<false, components::fixtures> : public basic_fixtures_synchronizer<false> {
	void rebuild_density(size_t) const;

	void reinference() const;

public:
	using basic_fixtures_synchronizer<false>::basic_fixtures_synchronizer;

	convex_poly_destruction_data& get_modifiable_destruction_data(const b2Fixture_index_in_component);

	void set_density(
		const float density, 
		const size_t collider_index = 0
	) const;

	void set_density_multiplier(
		const float multiplier,
		const size_t collider_index = 0
	) const;

	void set_friction(
		const float friction, 
		const size_t collider_index = 0
	) const;

	void set_restitution(
		const float restitution, 
		const size_t collider_index = 0
	) const;

	void set_physical_material(
		const assets::physical_material_id,
		const size_t collider_index = 0
	) const;

	void set_owner_body(const entity_id) const;

	component_synchronizer& operator=(const components::fixtures&);
	
	void set_offset(
		const colliders_offset_type, 
		const components::transform
	) const;

	void set_activated(bool) const;
};

template<>
class component_synchronizer<true, components::fixtures> : public basic_fixtures_synchronizer<true> {
public:
	using basic_fixtures_synchronizer<true>::basic_fixtures_synchronizer;
};