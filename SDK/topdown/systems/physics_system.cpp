#include "physics_system.h"
#include "entity_system/entity.h"

physics_system::physics_system() : accumulator(30.0, 1), 
	//world(b2Vec2(0.f, 0.f)) 
	b2world(b2Vec2(0.f, 9.8f)) {
	b2world.SetAllowSleeping(true);
	b2world.SetAutoClearForces(false);
}

void physics_system::process_entities(world& owner) {
	const unsigned steps = accumulator.update_and_extract_steps();

	/* we will update body's transforms HERE according to the ENTITY_MOVED message
	bitsquid's events are particularly well when message can be processed "within the same domain"
	whereas when it comes to registrations to entity-specific notations, there is probably some overhead imposed with searching the matching subscriber
	*/

	auto events = owner.get_message_queue<messages::moved_message>();

	for (auto it = events.begin(); it != events.end(); ++it) {
		auto physics = (*it).subject->find<components::physics>();
		auto transform = (*it).subject->find<components::transform>();

		if (!physics || !transform) continue;
		
		physics->body->SetTransform(transform->current.pos * PIXELS_TO_METERS, transform->current.rotation);
	}

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		reset_states();
		b2world.Step(accumulator.per_second(), velocityIterations, positionIterations);
	}

	/* note we DON'T send moved_message for every processed physics entity, because if something wants to keep track of an entity's position it can
	hold its reference and poll the position every frame */

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