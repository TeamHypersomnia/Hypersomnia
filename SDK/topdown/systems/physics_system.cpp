#include "physics_system.h"

physics_system::physics_system() : accumulator(30.0, 1), 
	//world(b2Vec2(0.f, 0.f)) 
	b2world(b2Vec2(0.f, 9.8f)) {
	b2world.SetAllowSleeping(true);
	b2world.SetAutoClearForces(false);
}

void physics_system::process_entities(world&) {
	const unsigned steps = accumulator.update_and_extract_steps();

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		reset_states();
		b2world.Step(accumulator.per_second(), velocityIterations, positionIterations);
	}

	b2world.ClearForces();
	smooth_states();
}

void physics_system::add(entity*) {

}

void physics_system::remove(entity* e) {
	b2world.DestroyBody(e->get<components::physics>().body);
}

void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();
		
		transform.current.pos = b->GetPosition();
		transform.current.pos *= METERS_TO_PIXELS;
		transform.current.rotation = b->GetAngle();

		transform.previous = transform.current;
	}
}

void physics_system::smooth_states() {
	const float one_minus_ratio = 1.f - accumulator.get_ratio();
 
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
 
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();
		transform.current.pos = vec2<float>(accumulator.get_ratio() * b->GetPosition()) + transform.previous.pos * PIXELS_TO_METERS * one_minus_ratio;
		transform.current.pos *= METERS_TO_PIXELS;
		transform.current.rotation = accumulator.get_ratio() * b->GetAngle() + one_minus_ratio * transform.previous.rotation;
	}
}