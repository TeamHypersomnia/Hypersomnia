#pragma once
#include "3rdparty/Box2D/Dynamics/b2Filter.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/propagate_const.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"

#include "game/messages/collision_message.h"

#include "game/detail/physics/physics_queries_declaration.h"
#include "game/detail/physics/colliders_connection.h"
#include "game/cosmos/get_corresponding.h"

class cosmos;
class physics_world_cache;

class b2Joint;
class b2Fixture;
class b2Body;
class b2World;

#if TODO_JOINTS
struct joint_cache {
	augs::propagate_const<b2Joint*> joint = nullptr;

	void clear(physics_world_cache&);

	bool is_constructed() const {
		return joint.get() != nullptr;
	}
};
#endif

struct physics_raycast_output {
	bool hit = false;
	vec2 intersection;
	vec2 normal;
	unversioned_entity_id what_entity;
};

class physics_world_cache {
	friend rigid_body_cache;
	friend colliders_cache;

#if TODO_JOINTS
	friend joint_cache;
	inferred_cache_map<joint_cache> joint_caches;
#endif

	template <class E>
	void specific_infer_colliders_from_scratch(
		const E&, 
		const colliders_connection&
	);

public:
	template <class E>
	struct concerned_with {
		static constexpr bool value = true;
	};

	// b2World on stack causes a stack overflow due to a large stack allocator, therefore it must be dynamically allocated
	std::unique_ptr<b2World> b2world;

	std::vector<messages::collision_message> accumulated_messages;

	physics_world_cache();
	~physics_world_cache();

	physics_world_cache(const physics_world_cache&);
	physics_world_cache& operator=(const physics_world_cache&);

	physics_world_cache& operator=(physics_world_cache&&) = delete;
	physics_world_cache(physics_world_cache&&) = delete;

	void clone_from(const physics_world_cache& source_world, cosmos& target_cosmos, const cosmos& source_cosmos);

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
	void for_each_intersection_with_circle_meters(Args&&... args) const {
		::for_each_intersection_with_circle_meters(get_b2world(), std::forward<Args>(args)...);
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

#if TODO_JOINTS
	joint_cache* find_joint_cache(const entity_id);
	const joint_cache* find_joint_cache(const entity_id) const;
#endif

	b2World& get_b2world() {
		return *b2world.get();
	}

	const b2World& get_b2world() const {
		return *b2world.get();
	}

	b2Fixture_index_in_component get_index_in_component(
		const b2Fixture& f, 
		const const_entity_handle&
	) const;

	void rechoose_owner_friction_body(entity_handle);
	void recurential_friction_handler(const logic_step, b2Body* const entity, b2Body* const friction_owner);

	void reserve_caches_for_entities(const size_t n);

	void infer_all(cosmos&);

	template <class E>
	void specific_infer_cache_for(const E&);

	template <class E>
	void specific_infer_rigid_body_existence(const E& handle);

	template <class E>
	void specific_infer_rigid_body(const E&);

	template <class E>
	void specific_infer_rigid_body_from_scratch(const E& handle);

	template <class E>
	void specific_infer_colliders(const E&);

	void infer_cache_for(const entity_handle&);
	void destroy_cache_of(const entity_handle&);

	void infer_colliders_from_scratch(const entity_handle&);
	void infer_colliders(const entity_handle&);
	void infer_rigid_body(const entity_handle&);

	void destroy_colliders_cache(const entity_handle&);
	void destroy_rigid_body_cache(const entity_handle&);

#if TODO_JOINTS
	void infer_joint(const entity_handle&);
	void destroy_joint_cache(const entity_handle&);

	template <class E>
	void specific_infer_joint(const E&);
#endif
};
