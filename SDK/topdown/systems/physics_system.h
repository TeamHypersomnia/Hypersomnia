#pragma once
#include "utility/delta_accumulator.h"
#include "entity_system/processing_system.h"

#include "../components/physics_component.h"
#include "../components/transform_component.h"

#include <functional>

using namespace augmentations;
using namespace entity_system;

class physics_system : public processing_system_templated<components::physics, components::transform> {
	util::delta_accumulator accumulator;

	void reset_states();
	void smooth_states();

	struct contact_listener : public b2ContactListener {
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

		world* world_ptr;
	};

	contact_listener listener;
public:
	std::function<void(world&)> substepping_routine;

	float timestep_multiplier;
	int enable_interpolation;
	int ray_casts_per_frame;

	b2World b2world;
	physics_system();

	void process_entities(world&) override;
	void add(entity*) override;
	void remove(entity*) override;
	void clear() override;

	struct raycast_output {
		vec2<> intersection, normal;
		bool hit;
		b2Fixture* what_fixture;
		entity* what_entity;

		raycast_output() : hit(false), what_fixture(nullptr) {}
	};

	std::vector<raycast_output> ray_cast_all_intersections(vec2<> p1_meters, vec2<> p2_meters, b2Filter filter, entity* ignore_entity = nullptr);
	
	struct edge_edge_output {
		vec2<> intersection;
		bool hit;
	};

	edge_edge_output edge_edge_intersection(vec2<> p1_meters, vec2<> p2_meters, vec2<> edge_p1, vec2<> edge_p2);

	raycast_output ray_cast(vec2<> p1_meters, vec2<> p2_meters, b2Filter filter, entity* ignore_entity = nullptr);
	raycast_output ray_cast_px(vec2<> p1, vec2<> p2, b2Filter filter, entity* ignore_entity = nullptr);
	
	vec2<> push_away_from_walls(vec2<> position, float radius, int ray_amount, b2Filter filter, entity* ignore_entity = nullptr);

	struct query_output {
		std::vector<b2Body*> bodies;
		query_output() {}
		query_output(const std::vector<b2Body*>& b) : bodies(b) {}
	};

	query_output query_square(vec2<> p1_meters, float side_meters, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);
	query_output query_square_px(vec2<> p1, float side, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);
	query_output query_aabb(vec2<> p1_meters, vec2<> p2_meters, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);
	query_output query_aabb_px(vec2<> p1, vec2<> p2, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);

	query_output query_body(augmentations::entity_system::entity&, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);

	query_output query_shape(b2Shape*, b2Filter* filter = nullptr, void* ignore_userdata = nullptr);
private:
	/* callback structure used in QueryAABB function to get all shapes near-by */
	struct query_aabb_input : b2QueryCallback {
		void* ignore_userdata;
		b2Filter* filter;
		std::set<b2Body*> output;
		std::vector<b2Fixture*> out_fixtures;

		query_aabb_input();
		bool ReportFixture(b2Fixture* fixture) override;
	};

	struct raycast_input : public b2RayCastCallback {
		entity* subject;
		b2Filter* subject_filter;
		
		bool save_all;
		raycast_output output;
		std::vector<raycast_output> outputs;

		bool ShouldRaycast(b2Fixture* fixture);
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
		raycast_input();
	};
};