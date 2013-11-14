#pragma once
#include "utility/delta_accumulator.h"
#include "entity_system/processing_system.h"

#include "../components/physics_component.h"
#include "../components/transform_component.h"

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
	std::vector<processing_system*> substepping_systems;
	float timestep_multiplier;

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

	raycast_output ray_cast   (vec2<> p1_meters, vec2<> p2_meters, b2Filter* filter = 0, entity* ignore_entity = nullptr);
	raycast_output ray_cast_px(vec2<> p1, vec2<> p2, b2Filter* filter = 0, entity* ignore_entity = nullptr);
private:
	struct raycast_input : public b2RayCastCallback {
		entity* subject;
		b2Filter* subject_filter;
		raycast_output output;

		bool ShouldRaycast(b2Fixture* fixture);
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
		raycast_input();
	};
};