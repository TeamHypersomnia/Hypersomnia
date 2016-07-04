#pragma once
#include <Box2D\Box2D.h>
#include "game/entity_id.h"
#include "game/entity_handle.h"

#include "game/components/physics_component.h"
#include "game/components/transform_component.h"
#include "game/detail/physics_engine_reflected_state.h"

#include <functional>
#include <set>

class cosmos;
class fixed_step;

class physics_system {
	struct rigid_body_black_box_detail {
		b2Body* body = nullptr;
	};

	struct colliders_black_box_detail {
		std::vector<std::vector<b2Fixture*>> fixtures_per_collider;
	};

	std::vector<colliders_black_box_detail> colliders_caches;
	std::vector<rigid_body_black_box_detail> rigid_body_caches;

	void reserve_caches_for_entities(size_t n);
	void construct(const_entity_handle);
	void destruct(const_entity_handle);

	friend class cosmos;
	friend class component_synchronizer<false, components::physics>;
	friend class component_synchronizer<true, components::physics>;
	friend class component_synchronizer<false, components::fixtures>;
	friend class component_synchronizer<true, components::fixtures>;

	bool is_constructed_rigid_body(const_entity_handle) const;
	bool is_constructed_colliders(const_entity_handle) const;

	rigid_body_black_box_detail& get_rigid_body_cache(const_entity_handle);
	colliders_black_box_detail& get_colliders_cache(const_entity_handle);
public:
	struct raycast_output {
		vec2 intersection, normal;
		bool hit = false;
		entity_id what_entity;
	};

	struct edge_edge_output {
		vec2 intersection;
		bool hit;
	};

	struct query_output {
		struct queried_result {
			b2Fixture* fixture;
			b2Vec2 location;

			bool operator<(const queried_result& b) const { return fixture < b.fixture; }
		};

		std::set<b2Body*> bodies;
		std::set<entity_id> entities;
		std::set<queried_result> details;

		query_output& operator+=(const query_output& b) {
			bodies.insert(b.bodies.begin(), b.bodies.end());
			entities.insert(b.entities.begin(), b.entities.end());
			details.insert(b.details.begin(), b.details.end());

			return *this;
		}
	};

	struct query_aabb_output {
		std::set<b2Body*> bodies;
		std::set<entity_id> entities;
		std::vector<b2Fixture*> fixtures;
	};

	physics_system();

	std::vector<raycast_output> ray_cast_all_intersections(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());

	edge_edge_output edge_edge_intersection(vec2 p1_meters, vec2 p2_meters, vec2 edge_p1, vec2 edge_p2);

	raycast_output ray_cast(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	raycast_output ray_cast_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id());
	
	vec2 push_away_from_walls(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());
	float get_closest_wall_intersection(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());

	query_aabb_output query_square(vec2 p1_meters, float side_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	query_aabb_output query_square_px(vec2 p1, float side, b2Filter filter, entity_id ignore_entity = entity_id());

	query_aabb_output query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	query_aabb_output query_aabb_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	query_output query_body(entity_id, b2Filter filter, entity_id ignore_entity = entity_id());

	query_output query_polygon(const std::vector<vec2>& vertices, b2Filter filter, entity_id ignore_entity = entity_id());
	query_output query_shape(b2Shape*, b2Filter filter, entity_id ignore_entity = entity_id());
	
	void step_and_set_new_transforms(fixed_step&);

	int ray_casts_since_last_step = 0;

	b2World b2world;
private:	
	/* callback structure used in QueryAABB function to get all shapes near-by */
	struct query_aabb_input : b2QueryCallback {
		entity_id ignore_entity;
		b2Filter filter;

		query_aabb_output out;

		bool ReportFixture(b2Fixture* fixture) override;
	};

	struct raycast_input : public b2RayCastCallback {
		const_entity_handle subject;
		raycast_input(const_entity_handle h) : subject(h) {}
		b2Filter subject_filter;

		bool save_all = false;
		raycast_output output;
		std::vector<raycast_output> outputs;

		bool ShouldRaycast(b2Fixture* fixture);
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
	};

	struct contact_listener : public b2ContactListener {
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

		std::vector<std::function<void()>> after_step_callbacks;
		cosmos* cosmos_ptr;
		fixed_step* step_ptr;
	};

	void rechoose_owner_friction_body(entity_handle);
	void recurential_friction_handler(entity_handle entity, entity_handle friction_owner);

	contact_listener listener;
};