#pragma once
#include <Box2D\Box2D.h>
#include "game/entity_id.h"

#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include <functional>
#include <set>

class cosmos;
class step_state;

class physics_system {
	cosmos& parent_cosmos;

public:
	struct raycast_output {
		vec2 intersection, normal;
		bool hit = false;
		entity_id what_entity;

		bool operator<(const raycast_output& b) const {
			return what_entity < b.what_entity;
		}
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

	physics_system(cosmos&);

	std::vector<raycast_output> ray_cast_all_intersections(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());

	edge_edge_output edge_edge_intersection(vec2 p1_meters, vec2 p2_meters, vec2 edge_p1, vec2 edge_p2);

	raycast_output ray_cast(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	raycast_output ray_cast_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id());
	
	vec2 push_away_from_walls(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());
	float get_closest_wall_intersection(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id());

	query_aabb_output query_square(vec2 p1_meters, float side_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	query_aabb_output query_square_px(vec2 p1, float side, b2Filter filter, entity_id ignore_entity = entity_id());

	query_aabb_output query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id());
	query_aabb_output query_aabb_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id());

	query_output query_body(entity_id, b2Filter filter, entity_id ignore_entity = entity_id());

	query_output query_polygon(const std::vector<vec2>& vertices, b2Filter filter, entity_id ignore_entity = entity_id());
	query_output query_shape(b2Shape*, b2Filter filter, entity_id ignore_entity = entity_id());
	
	void enable_listener(bool flag);
	
	void rechoose_owner_friction_body(entity_id);

	void react_to_new_entities(step_state&);
	void react_to_destroyed_entities(step_state&);

	void step_and_set_new_transforms(step_state&);

	entity_id get_owner_friction_field(entity_id);
	entity_id get_owner_body_entity(entity_id sub_entity);
	bool is_entity_physical(entity_id);
	bool are_connected_by_friction(entity_id child, entity_id parent);
	void resolve_density_of_associated_fixtures(entity_id);

	std::vector<b2Vec2> get_world_vertices(entity_id subject, bool meters = true, int fixture_num = 0);

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
		entity_id subject;
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
		step_state* step_ptr;
	};

	void recurential_friction_handler(entity_id entity, entity_id friction_owner);

	void set_transforms_from_body_transforms();

	contact_listener listener;
};