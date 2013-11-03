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

	b2World b2world;
	physics_system();

	void process_entities(world&) override;
	void add(entity*) override;
	void remove(entity*) override;
	void clear() override;
};