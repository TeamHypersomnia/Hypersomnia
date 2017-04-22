#include "game/systems_inferred/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/messages/collision_message.h"
#include "game/components/driver_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/flags_component.h"
#include "game/components/damage_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"

#include "physics_scripts.h"

#include "augs/graphics/renderer.h"
#include "augs/templates/container_templates.h"
#include "application/config_structs/debug_drawing_settings.h"

#define FRICTION_FIELDS_COLLIDE 0

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;
	
	const auto delta = cosm.get_fixed_delta();
	const auto now = cosm.get_timestamp();
	const auto si = cosmos.get_si();
	
	std::array<messages::collision_message, 2> msgs;

	for (int i = 0; i < 2; ++i) {
		auto& msg = msgs[i];

		const auto* fix_a = contact->GetFixtureA();
		const auto* fix_b = contact->GetFixtureB();

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

		const auto* const body_a = fix_a->GetBody();
		const auto* const body_b = fix_b->GetBody();

		msg.one_is_sensor = fix_a->IsSensor() || fix_b->IsSensor();
		msg.type = messages::collision_message::event_type::BEGIN_CONTACT;

		const auto subject = cosmos[fix_a->GetUserData()];
		const auto collider = cosmos[fix_b->GetUserData()];

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
				else if (collider_physics.dropped_or_created_cooldown.lasts(now, delta)) {
					collider_physics.dropped_or_created_cooldown = augs::stepped_cooldown();
					found_suitable = true;
				}
				else {
					for (int i = 0; i < 1; i++) {
						const b2Vec2 pointVelOther = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[i]);
						const auto velOtherPixels = si.get_pixels(vec2(pointVelOther));

						if (velOtherPixels.length() > 1) {
							const auto angle = vec2(worldManifold.normal).degrees_between(velOtherPixels);

							if (angle > 90) {
								found_suitable = true;
							}
						}

						auto& renderer = augs::renderer::get_current();

						if (renderer.debug.draw_friction_field_collisions_of_entering) {
							renderer.persistent_lines.draw_yellow(
								si.get_pixels(worldManifold.points[i]), 
								si.get_pixels(worldManifold.points[i]) + vec2(worldManifold.normal).set_length(150)
							);

							renderer.persistent_lines.draw_red(
								si.get_pixels(worldManifold.points[i]), 
								si.get_pixels(worldManifold.points[i]) + velOtherPixels
							);
						}
					}
				}

				if (found_suitable) {
					auto new_owner = subject_fixtures.get_owner_body().get_id();
					auto& grounds = collider_physics.owner_friction_grounds;
					
					friction_connection connection(new_owner);
					connection.fixtures_connected = 1;

					if (found_in(grounds, new_owner)) {
						auto found = find_in(grounds, new_owner);
						 LOG("Incr: %x", new_owner);

						connection.fixtures_connected = (*found).fixtures_connected + 1;
						grounds.erase(found);
					}
					else {
						 LOG("Reg: %x", new_owner);
					}
					
					grounds.push_back(connection);

					sys.rechoose_owner_friction_body(collider_fixtures.get_owner_body());
				}
			}
		}

		const auto collider_owner_body = collider.get_owner_body();
		const auto* const damage = subject.get_owner_body().find<components::damage>();
		const bool bullet_colliding_with_sender =
			damage != nullptr
			&& (
				damage->sender == collider_owner_body
				|| damage->sender_capability == collider_owner_body
			)
		;

		if (bullet_colliding_with_sender) {
			contact->SetEnabled(false);
			return;
		}

		msg.point = worldManifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
	}

	sys.accumulated_messages.push_back(msgs[0]);
	sys.accumulated_messages.push_back(msgs[1]);
}

void physics_system::contact_listener::EndContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;
	const auto si = cosmos.get_si();

	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1) {
			std::swap(fix_a, fix_b);
		}

		const auto* const body_a = fix_a->GetBody();
		const auto* const body_b = fix_b->GetBody();

		messages::collision_message msg;
		msg.type = messages::collision_message::event_type::END_CONTACT;

		const auto subject = cosmos[fix_a->GetUserData()];
		const auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = collider.get<components::fixtures>();

		auto& collider_physics = collider_fixtures.get_owner_body().get<components::special_physics>();

		if (subject_fixtures.is_friction_ground() && during_step) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it) {
					if ((*it).target == subject_fixtures.get_owner_body()) {
						auto& fixtures_connected = (*it).fixtures_connected;
						ensure(fixtures_connected > 0);

						--fixtures_connected;

						if (fixtures_connected == 0) {
							 LOG("Unreg: %x", subject_fixtures.get_owner_body().get_id());
							collider_physics.owner_friction_grounds.erase(it);
							sys.rechoose_owner_friction_body(collider_fixtures.get_owner_body());
						}
						else {
							 LOG("Decr: %x", subject_fixtures.get_owner_body().get_id());
						}

						break;
					}
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

	const auto si = cosmos.get_si();
	const auto delta = cosm.get_fixed_delta();
	const auto now = cosm.get_timestamp();

	std::array<messages::collision_message, 2> msgs;

	for (int i = 0; i < 2; ++i) {
		const auto* fix_a = contact->GetFixtureA();
		const auto* fix_b = contact->GetFixtureB();

		if (i == 1) {
			std::swap(fix_a, fix_b);
		}

		b2WorldManifold manifold;
		contact->GetWorldManifold(&manifold);

		const auto* const body_a = fix_a->GetBody();
		const auto* const body_b = fix_b->GetBody();

		auto& msg = msgs[i];

		msg.type = messages::collision_message::event_type::PRE_SOLVE;

		const auto subject = cosmos[fix_a->GetUserData()];
		const auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = subject.get<components::fixtures>();

		const const_entity_handle subject_owner_body = subject.get_owner_body();
		const const_entity_handle collider_owner_body = collider.get_owner_body();

		if (subject_fixtures.is_friction_ground()) {
			// friction fields do not collide with their children
			if (are_connected_by_friction(collider, subject)) {
				contact->SetEnabled(false);
				return;
			}

			auto& collider_physics = collider_owner_body.get<components::special_physics>();

			for (const auto& it : collider_physics.owner_friction_grounds) {
				if (it.target == subject_owner_body) {
					contact->SetEnabled(false);
					return;
				}
			}
		}

		const auto* const driver = subject_owner_body.find<components::driver>();
		
		const auto& collider_special_physics = collider_owner_body.get<components::special_physics>();

		const bool dropped_item_colliding_with_container =
			collider_special_physics.dropped_or_created_cooldown.lasts(now, delta)
			&& collider_special_physics.during_cooldown_ignore_collision_with == subject_owner_body
		;

		const bool colliding_with_owning_car = 
			driver 
			&& driver->owned_vehicle == collider_owner_body
		;

		// if (dropped_item_colliding_with_container) {
		// 	LOG(
		// 		"Ignoring collisiong between %x and %x", subject_owner_body, collider_owner_body
		// 	);
		// }

		if (
			dropped_item_colliding_with_container
			|| colliding_with_owning_car
		) {
			contact->SetEnabled(false);
			return;
		}
		
		const auto subject_owning_capability = subject_owner_body.get_owning_transfer_capability();
		const auto collider_owning_capability = collider.get_owning_transfer_capability();

		const bool fixtures_share_transfer_capability =
			subject_owning_capability.alive()
			&& collider_owning_capability.alive()
			&& subject_owning_capability == collider_owning_capability
		;

		if (fixtures_share_transfer_capability) {
			contact->SetEnabled(false);
			return;
		}

		if (
			subject_fixtures.standard_collision_resolution_disabled() 
			|| collider_fixtures.standard_collision_resolution_disabled()
		) {
			contact->SetEnabled(false);
		}

		msg.subject_b2Fixture_index = sys.get_index_in_component(fix_a, subject);
		msg.collider_b2Fixture_index = sys.get_index_in_component(fix_b, collider);

		msg.point = manifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	}

	sys.accumulated_messages.push_back(msgs[0]);
	sys.accumulated_messages.push_back(msgs[1]);
}

void physics_system::contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
	auto& sys = get_sys();
	auto& cosmos = cosm;
	const auto si = cosmos.get_si();

	messages::collision_message msgs[2];

	for (int i = 0; i < 2; ++i) {
		const auto* fix_a = contact->GetFixtureA();
		const auto* fix_b = contact->GetFixtureB();

		if (i == 1) {
			std::swap(fix_a, fix_b);
		}

		b2WorldManifold manifold;
		contact->GetWorldManifold(&manifold);

		const auto* const body_a = fix_a->GetBody();
		const auto* const body_b = fix_b->GetBody();

		auto& msg = msgs[i];

		msg.type = messages::collision_message::event_type::POST_SOLVE;

		const auto subject = cosmos[fix_a->GetUserData()];
		const auto collider = cosmos[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		auto& subject_fixtures = subject.get<components::fixtures>();
		auto& collider_fixtures = collider.get<components::fixtures>();

		msg.subject_b2Fixture_index = sys.get_index_in_component(fix_a, subject);
		msg.collider_b2Fixture_index = sys.get_index_in_component(fix_b, collider);

		msg.point = manifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);

		msg.normal_impulse = si.get_pixels(impulse->normalImpulses[0]);
		msg.tangent_impulse = si.get_pixels(impulse->tangentImpulses[0]);
	}

	sys.accumulated_messages.push_back(msgs[0]);
	sys.accumulated_messages.push_back(msgs[1]);
}
