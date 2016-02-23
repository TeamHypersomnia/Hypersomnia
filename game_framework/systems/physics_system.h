#pragma once
#include <Box2D\Box2D.h>
#include "entity_system/processing_system.h"
#include "entity_system/entity_id.h"

#include "../components/physics_component.h"
#include "../components/transform_component.h"

#include <functional>

using namespace augs;

class physics_system : public augs::event_only_system {
	void reset_states();

	struct contact_listener : public b2ContactListener {
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
		
		std::vector<std::function<void()>> after_step_callbacks;
		augs::world* world_ptr;
	};

	contact_listener listener;

	void recurential_friction_handler(entity_id entity, entity_id friction_owner);

public:
	using event_only_system::event_only_system;

	int ray_casts_per_frame = 0;

	bool enable_motors = true;
	void enable_listener(bool flag);

	b2World b2world;
	physics_system(world&);

	static void rechoose_owner_friction_body(augs::entity_id);
	void step_and_set_new_transforms();

	void destroy_fixtures_and_bodies();
	void destroy_whole_world();

	struct raycast_output {
		vec2 intersection, normal;
		bool hit;
		b2Fixture* what_fixture = nullptr;
		entity_id what_entity;

		raycast_output() : hit(false), what_fixture(nullptr) {}
	};

	std::vector<raycast_output> ray_cast_all_intersections(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	
	struct edge_edge_output {
		vec2 intersection;
		bool hit;
	};

	edge_edge_output edge_edge_intersection(vec2 p1_meters, vec2 p2_meters, vec2 edge_p1, vec2 edge_p2);

	raycast_output ray_cast(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	raycast_output ray_cast_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id());
	
	vec2 push_away_from_walls(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());
	float get_closest_wall_intersection(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());

	struct query_output {
		struct queried_result {
			b2Fixture* fixture;
			b2Vec2 location;

			bool operator<(const queried_result& b) const { return fixture < b.fixture; }
		};

		std::set<b2Body*> bodies;
		std::set<augs::entity_id> entities;
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
		std::set<augs::entity_id> entities;
		std::vector<b2Fixture*> fixtures;
	};

	query_aabb_output query_square(vec2 p1_meters, float side_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	query_aabb_output query_square_px(vec2 p1, float side, b2Filter filter, entity_id ignore_entity = entity_id());

	query_aabb_output query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	query_aabb_output query_aabb_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id());

	query_output query_body(augs::entity_id, b2Filter filter, entity_id ignore_entity = entity_id());

	query_output query_polygon(const std::vector<vec2>& vertices, b2Filter filter, entity_id ignore_entity = entity_id());
	query_output query_shape(b2Shape*, b2Filter filter, entity_id ignore_entity = entity_id());

	void create_bodies_and_fixtures_from_physics_definitions();
private:
	/* callback structure used in QueryAABB function to get all shapes near-by */
	struct query_aabb_input : b2QueryCallback {
		entity_id ignore_entity;
		b2Filter filter;
		
		query_aabb_output out;

		bool ReportFixture(b2Fixture* fixture) override;
	};

	struct raycast_input : public b2RayCastCallback {
		entity_id subject;
		b2Filter subject_filter;
		
		bool save_all = false;
		raycast_output output;
		std::vector<raycast_output> outputs;

		bool ShouldRaycast(b2Fixture* fixture);
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
	};
};