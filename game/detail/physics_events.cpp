#include "game/systems/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/messages/collision_message.h"
#include "game/components/driver_component.h"

#include "game/cosmos.h"
#include "game/step_state.h"

#include "graphics/renderer.h"

#define FRICTION_FIELDS_COLLIDE 0

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto& sys = this->cosmos_ptr->stateful_systems.get<physics_system>();

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

		/* collision messaging happens only for sensors here
		PreSolve is the counterpart for regular bodies
		*/

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;
		msg.type = messages::collision_message::event_type::BEGIN_CONTACT;

		msg.subject = static_cast<entity_id>(fix_a->GetUserData());
		msg.collider = static_cast<entity_id>(fix_b->GetUserData());

		auto& subject_fixtures = msg.subject->get<components::fixtures>();
		auto& collider_fixtures = msg.collider->get<components::fixtures>();

		if (subject_fixtures.is_friction_ground) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				auto& collider_physics = collider_fixtures.get_body_entity()->get<components::physics>();

				bool found_suitable = false;

				// always accept my own children
				if (sys.are_connected_by_friction(msg.collider, msg.subject)) {
					found_suitable = true;
				}
				else if (collider_physics.since_dropped.was_set && !sys.passed(collider_physics.since_dropped)) {
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
					collider_physics.owner_friction_grounds.push_back(subject_fixtures.get_body_entity());
					sys.rechoose_owner_friction_body(collider_fixtures.get_body_entity());
				}
			}
		}

		msg.point = worldManifold.points[0];
		msg.point *= METERS_TO_PIXELSf;

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		step_ptr->messages.post(msg);
	}
}

void physics_system::contact_listener::EndContact(b2Contact* contact) {
	auto& sys = this->cosmos_ptr->stateful_systems.get<physics_system>();

	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1)
			std::swap(fix_a, fix_b);

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;
		msg.type = messages::collision_message::event_type::END_CONTACT;

		msg.subject = static_cast<entity_id>(fix_a->GetUserData());
		msg.collider = static_cast<entity_id>(fix_b->GetUserData());

		auto& subject_fixtures = msg.subject->get<components::fixtures>();
		auto& collider_fixtures = msg.collider->get<components::fixtures>();

		auto& collider_physics = collider_fixtures.get_body_entity()->get<components::physics>();

		if (subject_fixtures.is_friction_ground) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it)
					if (*it == subject_fixtures.get_body_entity())
					{
						collider_physics.owner_friction_grounds.erase(it);
						sys.rechoose_owner_friction_body(collider_fixtures.get_body_entity());
						break;
					}
			}
		}

		msg.subject_impact_velocity = -body_a->GetLinearVelocity();
		msg.collider_impact_velocity = -body_b->GetLinearVelocity();
		step_ptr->messages.post(msg);
	}
}

void physics_system::contact_listener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	auto& sys = this->cosmos_ptr->stateful_systems.get<physics_system>();

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
		msg.subject = static_cast<entity_id>(fix_a->GetUserData());
		msg.collider = static_cast<entity_id>(fix_b->GetUserData());

		auto& subject_fixtures = msg.subject->get<components::fixtures>();
		auto& collider_fixtures = msg.collider->get<components::fixtures>();

		if (subject_fixtures.is_friction_ground) {
			// friction fields do not collide with their children
			if (sys.are_connected_by_friction(msg.collider, msg.subject)) {
				contact->SetEnabled(false);
				return;
			}

			auto& collider_physics = collider_fixtures.get_body_entity()->get<components::physics>();

			for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it)
				if (*it == subject_fixtures.get_body_entity())
				{
					contact->SetEnabled(false);
					return;
				}
		}

		auto* driver = sys.get_owner_body_entity(msg.subject)->find<components::driver>();

		bool colliding_with_owning_car = driver && driver->owned_vehicle == sys.get_owner_body_entity(msg.collider);

		if (colliding_with_owning_car) {
			contact->SetEnabled(false);
			return;
		}

		if (subject_fixtures.disable_standard_collision_resolution || collider_fixtures.disable_standard_collision_resolution) {
			contact->SetEnabled(false);
		}

		msg.point = manifold.points[0];
		msg.point *= METERS_TO_PIXELSf;

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	}

	step_ptr->messages.post(msgs[0]);
	step_ptr->messages.post(msgs[1]);
}

void physics_system::contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {

}
