#pragma once
#include <Box2D/Dynamics/b2Fixture.h>

#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/misc/enum_array.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/ids/physical_material_id.h"

#include "game/components/transform_component.h"

namespace components {
	struct fixtures {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::fixtures

		bool activated = true;
		bool is_friction_ground = false;
		bool disable_standard_collision_resolution = false;
		bool can_driver_shoot_through = false;

		assets::physical_material_id material = assets::physical_material_id::INVALID;

		float collision_sound_gain_mult = 1.f;

		float density = 1.f;
		float density_multiplier = 1.f;
		float friction = 0.f;
		float restitution = 0.f;

		b2Filter filter;
		bool destructible = false;
		bool sensor = false;

		augs::enum_array<components::transform, colliders_offset_type> offsets_for_created_shapes;

		entity_id owner_body;

		// END GEN INTROSPECTOR

		static components::transform transform_around_body(
			const const_entity_handle fixtures_entity, 
			const components::transform body_transform
		);
	};
}

class physics_system;
struct colliders_cache;

template <bool is_const>
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

	float get_density_multiplier() const;
	float get_friction() const;
	float get_restitution() const;
	float get_base_density() const;
	float get_density() const;
	
	ltrb get_local_aabb() const;

	bool is_destructible() const;
	bool is_friction_ground() const;
	bool standard_collision_resolution_disabled() const;
	bool can_driver_shoot_through() const;
	
	entity_id get_owner_body() const;
};

template<>
class component_synchronizer<false, components::fixtures> : public basic_fixtures_synchronizer<false> {
	void rebuild_density() const;

	void reinference() const;

public:
	using basic_fixtures_synchronizer<false>::basic_fixtures_synchronizer;

	void set_density(
		const float density
	) const;

	void set_density_multiplier(
		const float multiplier
	) const;

	void set_friction(
		const float friction
	) const;

	void set_restitution(
		const float restitution
	) const;

	void set_physical_material(
		const assets::physical_material_id
	) const;

	const component_synchronizer& operator=(const components::fixtures&) const;
	
	void set_offset(
		const colliders_offset_type, 
		const components::transform
	) const;

	void set_activated(bool) const;

	void set_owner_body(const entity_id) const;
};

template<>
class component_synchronizer<true, components::fixtures> : public basic_fixtures_synchronizer<true> {
public:
	using basic_fixtures_synchronizer<true>::basic_fixtures_synchronizer;
};