#include "augs/templates/container_templates.h"

#include "game/debug_drawing_settings.h"

#include "game/components/fixtures_component.h"
#include "game/components/driver_component.h"
#include "game/components/flags_component.h"
#include "game/components/missile_component.h"

#include "game/messages/collision_message.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/physics/contact_listener.h"

#include "game/inferred_caches/physics_world_cache.h"

#define FRICTION_FIELDS_COLLIDE 0

physics_world_cache& contact_listener::get_sys() const {
	return cosm.get_solvable_inferred({}).physics;
}

contact_listener::contact_listener(cosmos& cosm) : cosm(cosm) {
	get_sys().b2world->SetContactListener(this);
}

contact_listener::~contact_listener() {
	get_sys().b2world->SetContactListener(nullptr);
}

void contact_listener::BeginContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;
	
	const auto delta = cosm.get_fixed_delta();
	const auto now = cosm.get_timestamp();
	const auto si = cosmos.get_si();
	
	std::array<messages::collision_message, 2> msgs;

	bool post_collision_messages = true;

	for (int i = 0; i < 2; ++i) {
		auto& msg = msgs[i];

		const auto* fix_a = contact->GetFixtureA();
		const auto* fix_b = contact->GetFixtureB();

		const auto num_points = contact->GetManifold()->pointCount;
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		if (i == 1) {
			std::swap(fix_a, fix_b);

			if (num_points > 1) {
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

		ensure(subject.alive());
		ensure(collider.alive());

		const auto subject_fixtures = subject.get<invariants::fixtures>();
#if FRICTION_FIELDS_COLLIDE
		const auto collider_fixtures = collider.get<invariants::fixtures>();
#endif

		if (subject_fixtures.is_friction_ground()) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground())
#endif
			{
				auto& collider_physics = collider.get_owner_of_colliders().get_special_physics();

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

						if (DEBUG_DRAWING.draw_friction_field_collisions_of_entering) {
							DEBUG_PERSISTENT_LINES.emplace_back(yellow,
								vec2(si.get_pixels(worldManifold.points[i])),
								vec2(si.get_pixels(worldManifold.points[i])) + vec2(worldManifold.normal).set_length(150)
							);

							DEBUG_PERSISTENT_LINES.emplace_back(red,
								vec2(si.get_pixels(worldManifold.points[i])), 
								vec2(si.get_pixels(worldManifold.points[i])) + velOtherPixels
							);
						}
					}
				}

				if (found_suitable) {
					auto new_owner = subject.get_owner_of_colliders();
					auto& grounds = collider_physics.owner_friction_grounds;
					
					friction_connection connection;
					connection.target = new_owner;
					connection.fixtures_connected = 1;

					if (const auto found = find_in_if(
							grounds, 
							[new_owner](const auto candidate) { 
								return new_owner == candidate.target; 
							}
						);
						found != grounds.end()
					) {
						LOG("Incr: %x", new_owner);

						connection.fixtures_connected = (*found).fixtures_connected + 1;
						grounds.erase(found);
					}
					else {
						LOG("Reg: %x", new_owner);
					}
					
					grounds.emplace_back(connection);

					sys.rechoose_owner_friction_body(collider.get_owner_of_colliders());
				}
			}
		}

		//const auto collider_owner_body = collider.get_owner_of_colliders();
		//const auto* const damage = subject.get_owner_of_colliders().find<components::missile>();
		//
		//const bool bullet_colliding_with_sender =
		//	damage != nullptr
		//	&& (
		//		damage->sender == collider_owner_body
		//		|| cosmos[damage->sender].owning_transfer_capability_alive_and_same_as_of(collider_owner_body)
		//		|| damage->sender_capability == collider_owner_body
		//	)
		//;
		//
		//if (bullet_colliding_with_sender) {
		//	contact->SetEnabled(false);
		//	post_collision_messages = false;
		//	break;
		//}

		msg.point = worldManifold.points[0];
		msg.point = si.get_pixels(msg.point);
		msg.normal = si.get_pixels(worldManifold.normal);

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(worldManifold.points[0]);
	}

	if (post_collision_messages) {
		sys.accumulated_messages.push_back(msgs[0]);
		sys.accumulated_messages.push_back(msgs[1]);
	}
}

void contact_listener::EndContact(b2Contact* contact) {
	auto& sys = get_sys();
	auto& cosmos = cosm;

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

		ensure(subject.alive());
		ensure(collider.alive());

		msg.subject = subject;
		msg.collider = collider;

		const auto subject_fixtures = subject.get<invariants::fixtures>();
#if FRICTION_FIELDS_COLLIDE
		const auto collider_fixtures = collider.get<invariants::fixtures>();
#endif

		auto& collider_physics = collider.get_owner_of_colliders().get_special_physics();

		if (subject_fixtures.is_friction_ground() && during_step) {
#if FRICTION_FIELDS_COLLIDE
			if (!collider_fixtures.is_friction_ground)
#endif
			{
				for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it) {
					if ((*it).target == subject.get_owner_of_colliders()) {
						auto& fixtures_connected = (*it).fixtures_connected;
						ensure(fixtures_connected > 0);

						--fixtures_connected;

						if (fixtures_connected == 0) {
							 LOG("Unreg: %x", subject.get_owner_of_colliders().get_id());
							collider_physics.owner_friction_grounds.erase(it);
							sys.rechoose_owner_friction_body(collider.get_owner_of_colliders());
						}
						else {
							 LOG("Decr: %x", subject.get_owner_of_colliders().get_id());
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

void contact_listener::PreSolve(b2Contact* contact, const b2Manifold* /* oldManifold */) {
	auto& sys = get_sys();
	auto& cosmos = cosm;

	const auto si = cosmos.get_si();
	const auto delta = cosm.get_fixed_delta();
	const auto now = cosm.get_timestamp();

	std::array<messages::collision_message, 2> msgs;

	bool post_collision_messages = true;

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

		ensure(subject.alive());
		ensure(collider.alive());

		const auto subject_fixtures = subject.get<invariants::fixtures>();
		const auto collider_fixtures = subject.get<invariants::fixtures>();

		const const_entity_handle subject_owner_body = subject.get_owner_of_colliders();
		const const_entity_handle subject_capability = subject.get_owning_transfer_capability();
		const const_entity_handle collider_owner_body = collider.get_owner_of_colliders();

		if (subject_fixtures.is_friction_ground()) {
			// friction fields do not collide with their children
			if (are_connected_by_friction(collider, subject)) {
				contact->SetEnabled(false);
				post_collision_messages = false;
				break;
			}

			auto& collider_physics = collider_owner_body.get_special_physics();

			for (const auto& it : collider_physics.owner_friction_grounds) {
				if (it.target == subject_owner_body) {
					contact->SetEnabled(false);
					post_collision_messages = false;
					break;
				}
			}
		}

		if (subject_capability.alive()) {
			const auto* const driver = subject_capability.find<components::driver>();
			
			const bool colliding_with_driven_car = 
				driver != nullptr
				&& driver->owned_vehicle == collider_owner_body
			;

			if (colliding_with_driven_car) {
				contact->SetEnabled(false);
				post_collision_messages = false;
				break;
			}
		}
		
		const auto& collider_special_physics = collider_owner_body.get_special_physics();

		const bool dropped_item_colliding_with_container =
			collider_special_physics.dropped_or_created_cooldown.lasts(now, delta)
			&& collider_special_physics.during_cooldown_ignore_collision_with == subject_owner_body
		;

		// if (dropped_item_colliding_with_container) {
		// 	LOG(
		// 		"Ignoring collisiong between %x and %x", subject_owner_body, collider_owner_body
		// 	);
		// }

		if (dropped_item_colliding_with_container) {
			contact->SetEnabled(false);
			post_collision_messages = false;
			break;
		}
		
		const bool fixtures_share_transfer_capability =
			subject_owner_body.owning_transfer_capability_alive_and_same_as_of(collider)
		;

		if (fixtures_share_transfer_capability) {
			contact->SetEnabled(false);
			post_collision_messages = false;
			break;
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

	if (post_collision_messages) {
		sys.accumulated_messages.push_back(msgs[0]);
		sys.accumulated_messages.push_back(msgs[1]);
	}
}

void contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
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

		ensure(subject.alive());
		ensure(collider.alive());

		msg.subject_b2Fixture_index = sys.get_index_in_component(fix_a, subject);
		msg.collider_b2Fixture_index = sys.get_index_in_component(fix_b, collider);

		msg.point = manifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		
		const auto count = impulse->count;

		const auto* const normals = impulse->normalImpulses;
		const auto* const tangents = impulse->normalImpulses;

		msg.normal = si.get_pixels(manifold.normal);
		msg.normal_impulse = si.get_pixels(*std::max_element(normals, normals + count));
		msg.tangent_impulse = si.get_pixels(*std::max_element(tangents, tangents + count));
	}

	sys.accumulated_messages.push_back(msgs[0]);
	sys.accumulated_messages.push_back(msgs[1]);
}
