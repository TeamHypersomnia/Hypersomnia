#include "game/temporary_systems/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/messages/collision_message.h"
#include "game/components/driver_component.h"
#include "game/components/special_physics_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"

#include "physics_scripts.h"

#include "augs/graphics/renderer.h"

#define FRICTION_FIELDS_COLLIDE 0

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;
	auto delta = cosm.get_fixed_delta();

	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		int numPoints = contact->GetManifold()->pointCount;
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		if (i == 1) {
			std::swap(fix_a, fix_b);
			if (numPoints > 1) {
				std::swap(worldManifold.points[0], worldManifold.points[1]);
				std::swap(worldManifold.separations[0], worldManifold.separations[1]);
			}
			worldManifold.normal *= -1;
		}

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;

		if (fix_a->IsSensor() || fix_b->IsSensor())
			msg.one_is_sensor = true;

		msg.type = messages::collision_message::event_type::BEGIN_CONTACT;

		auto subject = cosmos[fix_a->GetUserData()];
		auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = collider.get<components::fixtures>();

		if (subject_fixtures.is_friction_ground()) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				auto& collider_physics = collider_fixtures.get_owner_body().get<components::special_physics>();

				bool found_suitable = false;

				// always accept my own children
				if (are_connected_by_friction(collider, subject)) {
					found_suitable = true;
				}
				else if (collider_physics.since_dropped.lasts(cosm.get_timestamp(), delta)) {
					collider_physics.since_dropped.unset();
					found_suitable = true;
				}
				else {
					for (int i = 0; i < 1; i++) {
						b2Vec2 pointVelOther = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[i]);
						auto velOtherPixels = vec2(pointVelOther) * METERS_TO_PIXELSf;

						if (velOtherPixels.length() > 1) {
							auto angle = vec2(worldManifold.normal).degrees_between(velOtherPixels);

							if (angle > 90)
								found_suitable = true;
						}

						if (renderer::get_current().debug_draw_friction_field_collisions_of_entering) {
							renderer::get_current().blink_lines.draw_yellow(METERS_TO_PIXELSf*worldManifold.points[i], METERS_TO_PIXELSf* worldManifold.points[i] + vec2(worldManifold.normal).set_length(150));
							renderer::get_current().blink_lines.draw_red(METERS_TO_PIXELSf*worldManifold.points[i], METERS_TO_PIXELSf* worldManifold.points[i] + velOtherPixels);
						}
					}
				}

				if (found_suitable) {
					auto new_owner = subject_fixtures.get_owner_body().get_id();
					auto& grounds = collider_physics.owner_friction_grounds;
					
					components::special_physics::friction_connection connection(new_owner);
					connection.fixtures_connected = 1;

					if (found_in(grounds, new_owner)) {
						auto found = find_in(grounds, new_owner);
						// LOG("Incr: %x", new_owner);

						connection.fixtures_connected = (*found).fixtures_connected + 1;
						grounds.erase(found);
					}
					else {
						// LOG("Reg: %x", new_owner);
					}
					
					grounds.push_back(connection);

					sys.rechoose_owner_friction_body(collider_fixtures.get_owner_body());
				}
			}
		}

		msg.point = worldManifold.points[0];
		msg.point *= METERS_TO_PIXELSf;

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		sys.accumulated_messages.push_back(msg);
	}
}

void physics_system::contact_listener::EndContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;

	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1)
			std::swap(fix_a, fix_b);

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;
		msg.type = messages::collision_message::event_type::END_CONTACT;

		auto subject = cosmos[fix_a->GetUserData()];
		auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = collider.get<components::fixtures>();

		auto& collider_physics = collider_fixtures.get_owner_body().get<components::special_physics>();

		if (subject_fixtures.is_friction_ground()) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it)
					if ((*it).target == subject_fixtures.get_owner_body())
					{
						auto& fixtures_connected = (*it).fixtures_connected;
						ensure(fixtures_connected > 0);

						--fixtures_connected;

						if (fixtures_connected == 0) {
							// LOG("Unreg: %x", subject_fixtures.get_owner_body().get_id());
							collider_physics.owner_friction_grounds.erase(it);
							sys.rechoose_owner_friction_body(collider_fixtures.get_owner_body());
						}
						else {
							// LOG("Decr: %x", subject_fixtures.get_owner_body().get_id());
						}

						break;
					}
			}
		}

		msg.subject_impact_velocity = -body_a->GetLinearVelocity();
		msg.collider_impact_velocity = -body_b->GetLinearVelocity();
		sys.accumulated_messages.push_back(msg);
	}
}

void physics_system::contact_listener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	auto& sys = get_sys();
	auto& cosmos = cosm;

	messages::collision_message msgs[2];

	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1)
			std::swap(fix_a, fix_b);

		b2WorldManifold manifold;
		contact->GetWorldManifold(&manifold);

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		auto& msg = msgs[i];

		msg.type = messages::collision_message::event_type::PRE_SOLVE;

		auto subject = cosmos[fix_a->GetUserData()];
		auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = collider.get<components::fixtures>();

		if (subject_fixtures.is_friction_ground()) {
			// friction fields do not collide with their children
			if (are_connected_by_friction(collider, subject)) {
				contact->SetEnabled(false);
				return;
			}

			auto& collider_physics = collider_fixtures.get_owner_body().get<components::special_physics>();

			for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it)
				if ((*it).target == subject_fixtures.get_owner_body())
				{
					contact->SetEnabled(false);
					return;
				}
		}

		auto* driver = subject.get_owner_body().find<components::driver>();

		bool colliding_with_owning_car = driver && driver->owned_vehicle == collider.get_owner_body();

		if (colliding_with_owning_car) {
			contact->SetEnabled(false);
			return;
		}

		if (subject_fixtures.standard_collision_resolution_disabled() || collider_fixtures.standard_collision_resolution_disabled()) {
			contact->SetEnabled(false);
		}

		msg.point = manifold.points[0];
		msg.point *= METERS_TO_PIXELSf;

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	}

	sys.accumulated_messages.push_back(msgs[0]);
	sys.accumulated_messages.push_back(msgs[1]);
}

void physics_system::contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {

}
