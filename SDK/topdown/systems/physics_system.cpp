#include "stdafx.h"
#include "physics_system.h"
#include "entity_system/entity.h"

#include "../components/damage_component.h"

#include "../messages/collision_message.h"

bool physics_system::raycast_input::ShouldRaycast(b2Fixture* fixture) {
	return
		(subject && reinterpret_cast<entity*>(fixture->GetBody()->GetUserData()) == subject) ||
		(b2ContactFilter::ShouldCollide(subject_filter, &fixture->GetFilterData()));
}

float32 physics_system::raycast_input::ReportFixture(b2Fixture* fixture, const b2Vec2& point,
	const b2Vec2& normal, float32 fraction) {
		output.intersection = point;

		output.hit = true;
		output.what_fixture = fixture;
		return fraction;
}

physics_system::raycast_input::raycast_input() : subject_filter(nullptr), subject(nullptr) {}

physics_system::physics_system() : accumulator(60.0, 5), timestep_multiplier(1.f),
	b2world(b2Vec2(0.f, 0.f)) {
		b2world.SetAllowSleeping(false);
		b2world.SetAutoClearForces(false);
		b2world.SetContactListener(&listener);
}

physics_system::raycast_output physics_system::ray_cast(vec2<> p1_meters, vec2<> p2_meters, b2Filter* filter, entity* ignore_entity) {
	raycast_input callback;
	callback.subject_filter = filter;
	callback.subject = ignore_entity;

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.output;
}

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto fix_a = contact->GetFixtureA();
	auto fix_b = contact->GetFixtureB();

	if (fix_a->IsSensor() || fix_b->IsSensor()) {
		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;

		msg.subject = static_cast<entity*>(body_a->GetUserData());
		msg.collider = static_cast<entity*>(body_b->GetUserData());

		if (fix_a->IsSensor()) {
			msg.impact_velocity = (body_b->GetLinearVelocity());
			world_ptr->post_message(msg);
		}

		if (fix_b->IsSensor()) {
			std::swap(msg.collider, msg.subject);
			msg.impact_velocity = (body_a->GetLinearVelocity());
			world_ptr->post_message(msg);
		}
	}
}

void physics_system::contact_listener::EndContact(b2Contact* contact) {
	auto fix_a = contact->GetFixtureA();
	auto fix_b = contact->GetFixtureB();

	if (fix_a->IsSensor() || fix_b->IsSensor()) {
		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;

		msg.subject = static_cast<entity*>(body_a->GetUserData());
		msg.collider = static_cast<entity*>(body_b->GetUserData());

		msg.sensor_end_contact = true;
		
		if (fix_a->IsSensor()) {
			msg.impact_velocity = -body_b->GetLinearVelocity();
			world_ptr->post_message(msg);
		}

		if (fix_b->IsSensor()) {
			std::swap(msg.collider, msg.subject);
			msg.impact_velocity = -body_a->GetLinearVelocity();
			world_ptr->post_message(msg);
		}
	}
}

void physics_system::contact_listener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	auto body_a = contact->GetFixtureA()->GetBody();
	auto body_b = contact->GetFixtureB()->GetBody();

	messages::collision_message msg;

	msg.subject = static_cast<entity*>(body_a->GetUserData());
	msg.collider = static_cast<entity*>(body_b->GetUserData());

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

	const unsigned steps = accumulator.update_and_extract_steps();

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		reset_states();
		
		for (auto& sys : substepping_systems)
			sys->substep(owner);

		b2world.Step(static_cast<float32>(accumulator.per_second()*timestep_multiplier), velocityIterations, positionIterations);
		b2world.ClearForces();
	}
	
	if(steps == 0) b2world.ClearForces();
	smooth_states();
}

void physics_system::add(entity*) {

}

void physics_system::clear() {
	std::vector<b2Body*> to_destroy;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) 
		to_destroy.push_back(b);

	for (auto& just_die : to_destroy)
		b2world.DestroyBody(just_die);

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
		transform.current.rotation *= 180.0f / 3.141592653589793238462f;

		transform.previous = transform.current;
	}
}

void physics_system::smooth_states() {
	const float ratio = static_cast<float>(accumulator.get_ratio());
 
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
 
		auto& transform = static_cast<entity*>(b->GetUserData())->get<components::transform>();

		transform.current.pos = transform.previous.pos + ratio * (METERS_TO_PIXELSf*b->GetPosition() - transform.previous.pos);
		transform.current.rotation = static_cast<float>(transform.previous.rotation + ratio * (b->GetAngle()*180.0/3.141592653589793238462 - transform.previous.rotation));
	}
}