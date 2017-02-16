#pragma once
#include "3rdparty/Box2D/Box2D.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/messages/collision_message.h"

#include "game/detail/convex_partitioned_shape.h"
#include "game/detail/physics_queries.h"
#include "augs/build_settings/setting_empty_bases.h"

#include <set>
#include <unordered_set>

class cosmos;
struct camera_cone;
#include "game/transcendental/step_declaration.h"

struct rigid_body_cache {
	b2Body* body = nullptr;
	std::vector<int> correspondent_colliders_caches;
};

struct colliders_cache {
	std::vector<std::vector<b2Fixture*>> fixtures_per_collider;
	int correspondent_rigid_body_cache = -1;
};

class EMPTY_BASES physics_system : public physics_queries<physics_system> {
	std::vector<colliders_cache> colliders_caches;
	std::vector<rigid_body_cache> rigid_body_caches;

	std::pair<size_t, size_t> map_fixture_pointer_to_indices(const b2Fixture* const f, const const_entity_handle);
	convex_partitioned_shape::convex_poly::destruction_data& map_fixture_pointer_to_convex_poly(const b2Fixture* const f, const entity_handle);

	void reserve_caches_for_entities(const size_t n);
	void fixtures_construct(const const_entity_handle);
	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	friend class cosmos;
	friend class physics_queries<physics_system>;
	friend class component_synchronizer<false, components::physics>;
	friend class component_synchronizer<true, components::physics>;
	friend class component_synchronizer<false, components::fixtures>;
	friend class component_synchronizer<true, components::fixtures>;
	template<bool> friend class basic_physics_synchronizer;
	template<bool> friend class basic_fixtures_synchronizer;

	bool is_constructed_rigid_body(const const_entity_handle) const;
	bool is_constructed_colliders(const const_entity_handle) const;

	rigid_body_cache& get_rigid_body_cache(const entity_id);
	colliders_cache& get_colliders_cache(const entity_id);
	const rigid_body_cache& get_rigid_body_cache(const entity_id) const;
	const colliders_cache& get_colliders_cache(const entity_id) const;

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

	raycast_output ray_cast(const vec2 p1_meters, const vec2 p2_meters, const b2Filter filter, const entity_id ignore_entity = entity_id()) const;
	raycast_output ray_cast_px(const vec2 p1, const vec2 p2, const b2Filter filter, const entity_id ignore_entity = entity_id()) const;
	
	vec2 push_away_from_walls(const vec2 position, const float radius, const int ray_amount, const b2Filter filter, const entity_id ignore_entity = entity_id()) const;
	float get_closest_wall_intersection(const vec2 position, const float radius, const int ray_amount, const b2Filter filter, const entity_id ignore_entity = entity_id()) const;

	void step_and_set_new_transforms(const logic_step);
	void post_and_clear_accumulated_collision_messages(const logic_step);

	mutable int ray_casts_since_last_step = 0;

	b2World& get_b2world() {
		return *b2world.get();
	}

	const b2World& get_b2world() const {
		return *b2world.get();
	}

	// b2world causes a stack overflow due to a large stack allocator, therefore it must be dynamically allocated
	std::unique_ptr<b2World> b2world;

	physics_system& operator=(const physics_system&);
	physics_system& operator=(physics_system&&) = delete;
	physics_system(const physics_system&) = delete;
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
