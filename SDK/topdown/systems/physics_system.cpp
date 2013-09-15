#include "physics_system.h"
#include "entity_system/entity.h"

#include "../components/damage_component.h"

#include "../messages/moved_message.h"
#include "../messages/collision_message.h"

physics_system::physics_system() : accumulator(60.0, 5), 
	b2world(b2Vec2(0.f, 9.8f)) {
	b2world.SetAllowSleeping(false);
	b2world.SetAutoClearForces(false);
	b2world.SetContactListener(&listener);
}

struct game_filter : public b2ContactFilter {
	bool ShouldCollide(b2Fixture* a, b2Fixture* b) override {
		if (b2ContactFilter::ShouldCollide(a, b)) {


		}
	}
};

void physics_system::contact_listener::BeginContact(b2Contact* contact) {

}

void physics_system::contact_listener::EndContact(b2Contact* contact) {

}

void physics_system::contact_listener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	messages::collision_message msg;

	auto body_a = contact->GetFixtureA()->GetBody();
	auto body_b = contact->GetFixtureB()->GetBody();

	msg.subject = static_cast<entity*>(body_a->GetUserData());
	msg.collider = static_cast<entity*>(body_b->GetUserData());
	
	if (msg.subject->find<components::damage>() || msg.collider->find<components::damage>()) 
		contact->SetEnabled(false);
	
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	msg.point = manifold.points[0];
	msg.point *= METERS_TO_PIXELSf;

	msg.impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	world_ptr->post_message(msg);

	std::swap(msg.collider, msg.subject);
	msg.impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	world_ptr->post_message(msg);
}

void physics_system::contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {

}

void physics_system::process_entities(world& owner) {
	listener.world_ptr = &owner;

	/* we will update body's transforms HERE according to the ENTITY_MOVED message
	bitsquid's events are particularly well when message can be processed "within the same domain"
	whereas when it comes to registrations to entity-specific notations, there is probably some overhead imposed with searching the matching subscriber
	*/
	/*
	auto events = owner.get_message_queue<messages::moved_message>();

	for (auto it = events.begin(); it != events.end(); ++it) {
		auto physics = (*it).subject->find<components::physics>();
		auto transform = (*it).subject->find<components::transform>();

		if (!physics || !transform) continue;
		
		transform->previous = transform->current;
		physics->body->SetTransform(transform->current.pos * PIXELS_TO_METERS, transform->current.rotation);
	}
	*/

	const unsigned steps = accumulator.update_and_extract_steps();

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		reset_states();
		b2world.Step(static_cast<float32>(accumulator.per_second()), velocityIterations, positionIterations);
	}

	/* note we DON'T send moved_message for every processed physics entity, because if something wants to keep track of an entity's position it can
	hold its reference and poll the position every frame */

	b2world.ClearForces();
	smooth_states();
}

void physics_system::add(entity*) {

}

void physics_system::clear() {
	auto it = b2world.GetBodyList();
	while (it) {
		auto next = it->GetNext();
		b2world.DestroyBody(it);
		it = next;
	}
	processing_system::clear();
}

void physics_system::remove(entity* e) {
	b2world.DestroyBody(e->get<components::physics>().body);
}

void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();
		
		transform.current.pos = b->GetPosition();
		transform.current.pos *= METERS_TO_PIXELSf;
		transform.current.rotation = b->GetAngle();

		transform.previous = transform.current;
	}
}

void physics_system::smooth_states() {
	const float ratio = static_cast<float>(accumulator.get_ratio());
 
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
 
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();

		transform.current.pos = transform.previous.pos + ratio * (METERS_TO_PIXELSf*b->GetPosition() - transform.previous.pos);
		transform.current.rotation = transform.previous.rotation + ratio * (b->GetAngle() - transform.previous.rotation);
	}
}