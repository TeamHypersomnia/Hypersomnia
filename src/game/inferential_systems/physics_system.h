#pragma once
#include <set>
#include <unordered_set>

#include "3rdparty/Box2D/Box2D.h"

#include "augs/misc/convex_partitioned_shape.h"

#include "game/assets/all_logical_assets_declarations.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/motor_joint_component.h"

#include "game/messages/collision_message.h"

class cosmos;
struct cosmos_common_state;

struct rigid_body_cache {
	b2Body* body = nullptr;
};

struct colliders_cache {
	augs::constant_size_vector<b2Fixture*, CONVEX_POLYS_COUNT> all_fixtures_in_component;
};

struct joint_cache {
	b2Joint* joint = nullptr;
};

class physics_system {
	std::vector<rigid_body_cache> rigid_body_caches;
	std::vector<colliders_cache> colliders_caches;
	std::vector<joint_cache> joint_caches;

	b2Fixture_index_in_component get_index_in_component(
		const b2Fixture* const f, 
		const const_entity_handle
	);
	
	void reserve_caches_for_entities(const size_t n);

	void create_inferred_state_for(const const_entity_handle);
	void create_inferred_state_for_fixtures(const const_entity_handle);
	void create_inferred_state_for_joint(const const_entity_handle);

	void destroy_inferred_state_of(const const_entity_handle);

	void create_additional_inferred_state(const cosmos_common_state&) {}
	void destroy_additional_inferred_state(const cosmos_common_state&) {}

	friend class cosmos;
	friend class component_synchronizer<false, components::rigid_body>;
	friend class component_synchronizer<true, components::rigid_body>;
	friend class component_synchronizer<false, components::motor_joint>;
	friend class component_synchronizer<true, components::motor_joint>;
	friend class component_synchronizer<false, components::fixtures>;
	friend class component_synchronizer<true, components::fixtures>;
	template <bool> friend class basic_physics_synchronizer;
	template <bool> friend class basic_fixtures_synchronizer;

	bool is_inferred_state_created_for_rigid_body(const const_entity_handle) const;
	bool is_inferred_state_created_for_colliders(const const_entity_handle) const;
	bool is_inferred_state_created_for_joint(const const_entity_handle) const;

	rigid_body_cache& get_rigid_body_cache(const entity_id);
	colliders_cache& get_colliders_cache(const entity_id);
	joint_cache& get_joint_cache(const entity_id);
	const rigid_body_cache& get_rigid_body_cache(const entity_id) const;
	const colliders_cache& get_colliders_cache(const entity_id) const;
	const joint_cache& get_joint_cache(const entity_id) const;

	std::vector<messages::collision_message> accumulated_messages;
public:
	struct raycast_output {
		bool hit = false;
		vec2 intersection;
		vec2 normal;
		unversioned_entity_id what_entity;
	};

	physics_system();

	std::vector<raycast_output> ray_cast_all_intersections(
		const vec2 p1_meters,
		const vec2 p2_meters, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	raycast_output ray_cast(
		const vec2 p1_meters, 
		const vec2 p2_meters, 
		const b2Filter filter, 
		const entity_id ignore_entity = entity_id()
	) const;

	raycast_output ray_cast_px(
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

	b2World& get_b2world() {
		return *b2world.get();
	}

	const b2World& get_b2world() const {
		return *b2world.get();
	}

	// b2World on stack causes a stack overflow due to a large stack allocator, therefore it must be dynamically allocated
	std::unique_ptr<b2World> b2world;

	physics_system(const physics_system&);
	physics_system& operator=(const physics_system&);

	physics_system& operator=(physics_system&&) = delete;
	physics_system(physics_system&&) = delete;
private:	
	struct raycast_input : public b2RayCastCallback {
		entity_id subject;
		b2Filter subject_filter;

		bool save_all = false;
		raycast_output output;
		std::vector<raycast_output> outputs;

		bool ShouldRaycast(b2Fixture* fixture) override;
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override;
	};

	struct contact_listener : public b2ContactListener {
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

		bool during_step = false;

		cosmos& cosm;
		physics_system& get_sys() const;

		contact_listener(const contact_listener&) = delete;
		contact_listener(contact_listener&&) = delete;
		
		contact_listener(cosmos&);
		~contact_listener();
		
		contact_listener& operator=(const contact_listener&) = delete;
		contact_listener& operator=(contact_listener&&) = delete;
	};

	void rechoose_owner_friction_body(entity_handle);
	void recurential_friction_handler(const logic_step, b2Body* const entity, b2Body* const friction_owner);
};
