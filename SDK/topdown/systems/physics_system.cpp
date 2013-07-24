#include "physics_system.h"

physics_system::physics_system() : accumulator(30.0, 1), 
	//world(b2Vec2(0.f, 0.f)) 
	world(b2Vec2(0.f, 9.8f)) 
{
	world.SetAllowSleeping(true);
	world.SetAutoClearForces(true);
}

void physics_system::process_entities() {
	const unsigned steps = accumulator.update_and_extract_steps();

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 1;
		int32 positionIterations = 1;

		reset_states();
		world.Step(1.0 / 30.0, 1, 1);
	}

	world.ClearForces();
	smooth_states();
}

void physics_system::reset_states() {
	for (b2Body* b = world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		//if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();
		
		transform.current.pos = b->GetPosition();
		transform.current.rotation = b->GetAngle();
		transform.previous = transform.current;
	}
}

void physics_system::smooth_states() {
	const float one_minus_ratio = 1.f - accumulator.get_ratio();
 
	for (b2Body * b = world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
 
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();
		transform.current.pos = vec2<float>(accumulator.get_ratio() * b->GetPosition()) + transform.previous.pos * one_minus_ratio;
		transform.current.rotation = accumulator.get_ratio() * b->GetAngle() + one_minus_ratio * transform.previous.rotation;
	}
}


void physics_system::add(entity* e) {
	processing_system_templated::add(e);
}

void physics_system::remove(entity* e) {
	processing_system_templated::remove(e);
}