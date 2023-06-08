#include "augs/templates/container_templates.h"

#include "game/debug_drawing_settings.h"

#include "game/components/fixtures_component.h"
#include "game/components/driver_component.h"
#include "game/components/flags_component.h"
#include "game/components/missile_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/messages/collision_message.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/physics/contact_listener.h"

#include "game/detail/physics/missile_surface_info.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/melee/like_melee.h"
#include "game/detail/sentience/sentience_getters.h"

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

	const auto si = cosm.get_si();
	const auto clk = cosm.get_clock();	
	
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

		const auto subject = cosm[fix_a->GetUserData()];
		const auto collider = cosm[fix_b->GetUserData()];

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
				else if (collider_physics.dropped_or_created_cooldown.lasts(clk)) {
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

#if TODO_CARS
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
#else
				(void)found_suitable;
#endif
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

		msg.subject_impact_velocity = si.get_pixels(body_a->GetLinearVelocity());
		msg.collider_impact_velocity = si.get_pixels(body_b->GetLinearVelocity());
	}

	if (post_collision_messages) {
		sys.accumulated_messages.push_back(msgs[0]);
		sys.accumulated_messages.push_back(msgs[1]);
	}
}

void contact_listener::EndContact(b2Contact* contact) {
	auto& sys = get_sys();
	const auto si = cosm.get_si();

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

		const auto subject = cosm[fix_a->GetUserData()];
		const auto collider = cosm[fix_b->GetUserData()];

		ensure(subject.alive());
		ensure(collider.alive());

		msg.subject = subject;
		msg.collider = collider;

		const auto subject_fixtures = subject.get<invariants::fixtures>();
#if FRICTION_FIELDS_COLLIDE
		const auto collider_fixtures = collider.get<invariants::fixtures>();
#endif

		if (const auto collider_owner = collider.get_owner_of_colliders()) {
			auto& collider_physics = collider_owner.get_special_physics();

			if (subject_fixtures.is_friction_ground() && during_step) {
	#if FRICTION_FIELDS_COLLIDE
				if (!collider_fixtures.is_friction_ground)
	#endif
				{
	#if TODO_CARS
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
	#else
					(void)collider_physics;
	#endif
				}
			}
		}

		msg.subject_impact_velocity = si.get_pixels(body_a->GetLinearVelocity());
		msg.collider_impact_velocity = si.get_pixels(body_b->GetLinearVelocity());
		sys.accumulated_messages.push_back(msg);
	}
}

void contact_listener::PreSolve(b2Contact* contact, const b2Manifold* /* oldManifold */) {
	auto& sys = get_sys();

	const auto si = cosm.get_si();
	const auto clk = cosm.get_clock();
	const auto& now = clk.now;

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

		const auto subject = cosm[fix_a->GetUserData()];
		const auto collider = cosm[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		ensure(subject.alive());
		ensure(collider.alive());

		const auto subject_fixtures = subject.get<invariants::fixtures>();
		const auto collider_fixtures = subject.get<invariants::fixtures>();

		const const_entity_handle subject_owner_body = subject.get_owner_of_colliders();
		const const_entity_handle subject_capability = subject.get_owning_transfer_capability();
		const const_entity_handle collider_owner_body = collider.get_owner_of_colliders();

		ensure(subject_owner_body.alive());
		ensure(collider_owner_body.alive());

		if (subject_owner_body.dead() || collider_owner_body.dead()) {
			return;
		}

#if TODO_CARS
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
#endif

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

		const bool dropped_item_colliding_with_container = [&]() {
			if (collider_special_physics.dropped_or_created_cooldown.lasts(clk)) {
				if (collider_special_physics.during_cooldown_ignore_other_cooled_down) {
					const auto& subject_special_physics = subject_owner_body.get_special_physics();

					if (subject_special_physics.during_cooldown_ignore_other_cooled_down 
						&& subject_special_physics.dropped_or_created_cooldown.lasts(clk)
					) {
						return true;
					}
				}

				const auto& ignored = collider_special_physics.during_cooldown_ignore_collision_with;

				if (ignored.is_set()) {
					if (ignored == subject_owner_body) {
						return true;
					}

					if (ignored == subject_capability) {
						return true;
					}
				}
			}

			return false;
		}();

		 /* if (dropped_item_colliding_with_container) { */
		 /* 	LOG( */
		 /* 		"Ignoring collisiong between %x and %x", subject_owner_body, collider_owner_body */
		 /* 	); */
		 /* } */

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

		if (subject_fixtures.ignore_standard_collision_resolution() 
			|| collider_fixtures.ignore_standard_collision_resolution()
		) {
			contact->SetEnabled(false);
		}

		if (subject.has<components::missile>() || is_like_thrown_melee(subject) || is_like_melee_in_action(subject)) {
			const auto info = missile_surface_info(subject, collider);

			if (info.ignore_standard_collision_resolution() || collider.has<components::sentience>()) {
				contact->SetEnabled(false);
				post_collision_messages = false;
				break;
			}
		}

		if (const auto missile = subject.find<components::missile>()) {
			const bool ricochet_cooldown = missile->when_last_ricocheted.was_set() && now.step <= missile->when_last_ricocheted.step + 1;

			if (ricochet_cooldown) {
				contact->SetEnabled(false);
				post_collision_messages = false;
				break;
			}
		}

		msg.indices.subject = sys.get_index_in_component(*fix_a, subject);
		msg.indices.collider = sys.get_index_in_component(*fix_b, collider);

		msg.normal = si.get_pixels(manifold.normal);
		msg.point = manifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = si.get_pixels(body_a->GetLinearVelocity());
		msg.collider_impact_velocity = si.get_pixels(body_b->GetLinearVelocity());
	}

	if (post_collision_messages) {
		sys.accumulated_messages.push_back(msgs[0]);
		sys.accumulated_messages.push_back(msgs[1]);
	}
}

void contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
	auto& sys = get_sys();
	const auto si = cosm.get_si();

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

		const auto subject = cosm[fix_a->GetUserData()];
		const auto collider = cosm[fix_b->GetUserData()];

		msg.subject = subject;
		msg.collider = collider;

		ensure(subject.alive());
		ensure(collider.alive());

		msg.indices.subject = sys.get_index_in_component(*fix_a, subject);
		msg.indices.collider = sys.get_index_in_component(*fix_b, collider);

		msg.point = manifold.points[0];
		msg.point = si.get_pixels(msg.point);

		msg.subject_impact_velocity = si.get_pixels(body_a->GetLinearVelocity());
		msg.collider_impact_velocity = si.get_pixels(body_b->GetLinearVelocity());
		
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
