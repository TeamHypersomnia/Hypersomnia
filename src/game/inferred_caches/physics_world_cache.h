#pragma once
#include <set>
#include <unordered_set>

#include "3rdparty/Box2D/Box2D.h"

#include "augs/templates/propagate_const.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/messages/collision_message.h"

#include "game/detail/physics/physics_queries_declaration.h"
#include "game/detail/physics/colliders_connection.h"

class cosmos;

struct rigid_body_cache {
	augs::propagate_const<b2Body*> body = nullptr;

	bool is_constructed() const {
		return body.get() != nullptr;
	}
};

struct colliders_cache {
	augs::constant_size_vector<
		augs::propagate_const<b2Fixture*>, 
		CONVEX_POLYS_COUNT
	> all_fixtures_in_component;

	colliders_connection connection;

	bool is_constructed() const {
		return all_fixtures_in_component.size() > 0;
	}
};

struct joint_cache {
	augs::propagate_const<b2Joint*> joint = nullptr;

	bool is_constructed() const {
		return joint.get() != nullptr;
	}
};

struct physics_raycast_output {
	bool hit = false;
	vec2 intersection;
	vec2 normal;
	unversioned_entity_id what_entity;
};

class physics_world_cache {
	std::vector<rigid_body_cache> rigid_body_caches;
	std::vector<colliders_cache> colliders_caches;
	std::vector<joint_cache> joint_caches;


public:
	// b2World on stack causes a stack overflow due to a large stack allocator, therefore it must be dynamically allocated
	std::unique_ptr<b2World> b2world;

	std::vector<messages::collision_message> accumulated_messages;

	physics_world_cache();

	physics_world_cache(const physics_world_cache&);
	physics_world_cache& operator=(const physics_world_cache&);

	physics_world_cache& operator=(physics_world_cache&&) = delete;
	physics_world_cache(physics_world_cache&&) = delete;

	std::vector<physics_raycast_output> ray_cast_all_intersections(
		const vec2 p1_meters,
		const vec2 p2_meters, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	physics_raycast_output ray_cast(
		const vec2 p1_meters, 
		const vec2 p2_meters, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	physics_raycast_output ray_cast_px(
		const si_scaling si,
		const vec2 p1, 
		const vec2 p2, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;
	
	vec2 push_away_from_walls(
		const si_scaling, 
		const vec2 position, 
		const float radius, 
		const int ray_amount, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	float get_closest_wall_intersection(
		const si_scaling, 
		const vec2 position, 
		const float radius, 
		const int ray_amount, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	/* Interface for physics queries */

	template <class... Args>
	void for_each_in_aabb_meters(Args&&... args) const {
		::for_each_in_aabb_meters(get_b2world(), std::forward<Args>(args)...);
	}

	template <class... Args>
	void for_each_intersection_with_shape_meters(Args&&... args) const {
		::for_each_intersection_with_shape_meters(get_b2world(), std::forward<Args>(args)...);
	}

	template <class... Args>
	void for_each_in_aabb(Args&&... args) const {
		::for_each_in_aabb(get_b2world(), std::forward<Args>(args)...);
	}

	template <class... Args>
	void for_each_in_camera(Args&&... args) const {
		::for_each_in_camera(get_b2world(), std::forward<Args>(args)...);
	}

	template <class... Args>
	void for_each_intersection_with_triangle(Args&&... args) const {
		::for_each_intersection_with_triangle(get_b2world(), std::forward<Args>(args)...);
	}

	template <class... Args>
	void for_each_intersection_with_polygon(Args&&... args) const {
		::for_each_intersection_with_polygon(get_b2world(), std::forward<Args>(args)...);
	}

	void step_and_set_new_transforms(const logic_step);
	void post_and_clear_accumulated_collision_messages(const logic_step);

	mutable std::size_t ray_casts_since_last_step = 0u;

	rigid_body_cache& get_rigid_body_cache(const entity_id);
	colliders_cache& get_colliders_cache(const entity_id);
	joint_cache& get_joint_cache(const entity_id);

	const rigid_body_cache& get_rigid_body_cache(const entity_id) const;
	const colliders_cache& get_colliders_cache(const entity_id) const;
	const joint_cache& get_joint_cache(const entity_id) const;

	b2World& get_b2world() {
		return *b2world.get();
	}

	const b2World& get_b2world() const {
		return *b2world.get();
	}

	b2Fixture_index_in_component get_index_in_component(
		const b2Fixture* const f, 
		const const_entity_handle
	) const;

	void rechoose_owner_friction_body(entity_handle);
	void recurential_friction_handler(const logic_step, b2Body* const entity, b2Body* const friction_owner);

	void reserve_caches_for_entities(const size_t n);

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);

	void infer_rigid_body_existence(const const_entity_handle);

	void infer_cache_for_colliders(const const_entity_handle);
	void infer_cache_for_rigid_body(const const_entity_handle);
	void infer_cache_for_joint(const const_entity_handle);

	void destroy_colliders_cache(const const_entity_handle);
	void destroy_rigid_body_cache(const const_entity_handle);
	void destroy_joint_cache(const const_entity_handle);
};
