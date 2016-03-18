#include "physics_system.h"
double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/new_entity_message.h"

#include "../detail/physics_setup_helpers.h"
#include "../components/physics_definition_component.h"

#include "../components/fixtures_component.h"

#include "log.h"

using namespace augs;

bool physics_system::raycast_input::ShouldRaycast(b2Fixture* fixture) {
	entity_id fixture_entity = fixture->GetBody()->GetUserData();
	return
		(!subject || fixture_entity != subject) &&
		(b2ContactFilter::ShouldCollide(&subject_filter, &fixture->GetFilterData()));
}

float32 physics_system::raycast_input::ReportFixture(b2Fixture* fixture, const b2Vec2& point,
	const b2Vec2& normal, float32 fraction) {
	output.intersection = point;

	output.hit = true;
	output.what_fixture = fixture;
	output.what_entity = fixture->GetBody()->GetUserData();
	output.normal = normal;

	if (save_all) {
		outputs.push_back(output);
		return 1.f;
	}

	return fraction;
}

physics_system::physics_system(world& parent_world) : event_only_system(parent_world),
b2world(b2Vec2(0.f, 0.f)), ray_casts_per_frame(0) {
	b2world.SetAllowSleeping(false);
	b2world.SetAutoClearForces(false);
	enable_listener(true);
}

std::vector<physics_system::raycast_output> physics_system::ray_cast_all_intersections
(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity) {
	++ray_casts_per_frame;

	raycast_input callback;
	callback.subject_filter = filter;
	callback.subject = ignore_entity;
	callback.save_all = true;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
	}

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.outputs;
}

physics_system::edge_edge_output physics_system::edge_edge_intersection(vec2 p1_meters, vec2 p2_meters, vec2 edge_p1, vec2 edge_p2) {
	/* prepare b2RayCastOutput/b2RayCastInput data for raw b2EdgeShape::RayCast call */
	b2RayCastOutput output;
	b2RayCastInput input;
	output.fraction = 0.f;
	input.maxFraction = 1.0;
	input.p1 = p1_meters;
	input.p2 = p2_meters;

	/* we don't need to transform edge or ray since they are in the same space
	but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
	*/
	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

	b2EdgeShape b2edge;
	b2edge.Set(b2Vec2(edge_p1), b2Vec2(edge_p2));

	edge_edge_output out;
	out.hit = b2edge.RayCast(&output, input, null_transform, 0);
	out.intersection = input.p1 + output.fraction * (input.p2 - input.p1);

	return out;
}

float physics_system::get_closest_wall_intersection(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity) {
	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		auto out = ray_cast_px(position, position + vec2().set_from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
		}
	}

	return worst_distance;
}

vec2 physics_system::push_away_from_walls(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity) {
	vec2 resultant;

	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		auto out = ray_cast_px(position, position + vec2().set_from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
			resultant += diff;
		}
	}

	if (resultant.non_zero())
		return position + (-resultant).set_length(radius - worst_distance);
	else return position;
}

physics_system::raycast_output physics_system::ray_cast(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity) {
	++ray_casts_per_frame;

	raycast_input callback;
	callback.subject_filter = filter;
	callback.subject = ignore_entity;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
		return callback.output;
	}

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.output;
}

physics_system::raycast_output physics_system::ray_cast_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity) {
	auto out = ray_cast(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
	out.intersection *= METERS_TO_PIXELSf;

	return out;
}

bool physics_system::query_aabb_input::ReportFixture(b2Fixture* fixture) {
	if ((b2ContactFilter::ShouldCollide(&filter, &fixture->GetFilterData()))
		&& fixture->GetBody()->GetUserData() != ignore_entity) {
		out.bodies.insert(fixture->GetBody());
		out.fixtures.push_back(fixture);
		out.entities.insert(fixture->GetUserData());
	}

	return true;
}

physics_system::query_aabb_output physics_system::query_square(vec2 p1_meters, float side_meters, b2Filter filter, entity_id ignore_entity) {
	b2AABB aabb;
	aabb.lowerBound = p1_meters - side_meters / 2;
	aabb.upperBound = p1_meters + side_meters / 2;
	return query_aabb(aabb.lowerBound, aabb.upperBound, filter, ignore_entity);
}

physics_system::query_aabb_output physics_system::query_square_px(vec2 p1, float side, b2Filter filter, entity_id ignore_entity) {
	return query_square(p1 * PIXELS_TO_METERSf, side * PIXELS_TO_METERSf, filter, ignore_entity);
}

physics_system::query_aabb_output physics_system::query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity) {
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;
	b2AABB aabb;
	aabb.lowerBound = p1_meters;
	aabb.upperBound = p2_meters;

	b2world.QueryAABB(&callback, aabb);

	return callback.out;
}

physics_system::query_output physics_system::query_body(augs::entity_id subject, b2Filter filter, entity_id ignore_entity) {
	query_output total_output;

	for (b2Fixture* f = subject->get<components::physics>().body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
		auto world_vertices = get_world_vertices(subject, true);

		b2PolygonShape shape;
		shape.Set(world_vertices.data(), world_vertices.size());

		total_output += query_shape(&shape, filter, ignore_entity);
	}

	return total_output;
}

physics_system::query_output physics_system::query_polygon(const std::vector<vec2>& vertices, b2Filter filter, entity_id ignore_entity) {
	b2PolygonShape poly_shape;
	std::vector<b2Vec2> verts;

	for (auto& v : vertices)
		verts.push_back(PIXELS_TO_METERSf * b2Vec2(v.x, v.y));

	poly_shape.Set(verts.data(), verts.size());
	return query_shape(&poly_shape, filter, ignore_entity);
}

physics_system::query_output physics_system::query_shape(b2Shape* shape, b2Filter filter, entity_id ignore_entity) {
	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;

	b2AABB shape_aabb;
	shape->ComputeAABB(&shape_aabb, null_transform, 0);
	b2world.QueryAABB(&callback, shape_aabb);

	physics_system::query_output out;

	for (auto fixture : callback.out.fixtures) {
		auto result = b2TestOverlapInfo(shape, 0, fixture->GetShape(), 0, null_transform, fixture->GetBody()->GetTransform());
		if (result.overlap) {
			out.bodies.insert(fixture->GetBody());
			out.entities.insert(fixture->GetUserData());
			out.details.insert({ fixture, result.pointA });
		}
	}

	return out;
}

physics_system::query_aabb_output physics_system::query_aabb_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity) {
	return query_aabb(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
}

#include "game_framework/components/movement_component.h"
#include "graphics/renderer.h"

#define FRICTION_FIELDS_COLLIDE 0

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto& sys = this->world_ptr->get_system<physics_system>();

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
				if (components::physics::are_connected_by_friction(msg.collider, msg.subject)) {
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
							auto angle = vec2(worldManifold.normal).angle_between(velOtherPixels);

							if (std::abs(angle) > 90)
								found_suitable = true;
						}

						renderer::get_current().blink_lines.draw_yellow(METERS_TO_PIXELSf*worldManifold.points[i], METERS_TO_PIXELSf* worldManifold.points[i] + vec2(worldManifold.normal).set_length(150));
						renderer::get_current().blink_lines.draw_red(METERS_TO_PIXELSf*worldManifold.points[i], METERS_TO_PIXELSf* worldManifold.points[i] + velOtherPixels);
					}
				}

				if (found_suitable) {
					collider_physics.owner_friction_grounds.push_back(subject_fixtures.get_body_entity());
					physics_system::rechoose_owner_friction_body(collider_fixtures.get_body_entity());
				}
			}
		}

		if (fix_a->IsSensor() || fix_b->IsSensor()) {
			msg.subject_impact_velocity = (body_a->GetLinearVelocity());
			msg.collider_impact_velocity = (body_b->GetLinearVelocity());
			world_ptr->post_message(msg);
		}
	}
}

void physics_system::contact_listener::EndContact(b2Contact* contact) {
	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1)
			std::swap(fix_a, fix_b);

		auto body_a = fix_a->GetBody();
		auto body_b = fix_b->GetBody();

		messages::collision_message msg;

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
					physics_system::rechoose_owner_friction_body(collider_fixtures.get_body_entity());
					break;
				}
			}
		}

		if (fix_a->IsSensor() || fix_b->IsSensor()) {
			msg.sensor_end_contact = true;
			msg.subject_impact_velocity = -body_a->GetLinearVelocity();
			msg.collider_impact_velocity = -body_b->GetLinearVelocity();
			world_ptr->post_message(msg);
		}
	}
}

#include "../components/driver_component.h"

void physics_system::contact_listener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
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

		msg.subject = static_cast<entity_id>(fix_a->GetUserData());
		msg.collider = static_cast<entity_id>(fix_b->GetUserData());

		auto& subject_fixtures = msg.subject->get<components::fixtures>();
		auto& collider_fixtures = msg.collider->get<components::fixtures>();

		if (subject_fixtures.is_friction_ground) {
			// friction fields do not collide with their children
			if (components::physics::are_connected_by_friction(msg.collider, msg.subject)) {
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

		auto* maybe_driver = components::physics::get_owner_body_entity(msg.subject)->find<components::driver>();

		if (maybe_driver) {
			/* do not collide with the car I currently ride to avoid strange artifacts */
			if (maybe_driver->owned_vehicle == components::physics::get_owner_body_entity(msg.collider)) {
				contact->SetEnabled(false);
				return;
			}
		}

		msg.point = manifold.points[0];
		msg.point *= METERS_TO_PIXELSf;

		msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
		msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	}

	world_ptr->post_message(msgs[0]);
	world_ptr->post_message(msgs[1]);
}

void physics_system::contact_listener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {

}

template <class T>
T constrainAngle(T x) {
	x = static_cast<T>(fmod(x + 180, 360));
	if (x < 0)
		x += 360;
	return x - 180;
}

void physics_system::enable_listener(bool flag) {
	b2world.SetContactListener(flag ? &listener : nullptr);
}
#include "graphics/renderer.h"

void physics_system::step_and_set_new_transforms() {
	parent_world.get_message_queue<messages::collision_message>().clear();

	listener.world_ptr = &parent_world;

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;

		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();
		physics.measured_carried_mass = 0.f;

		b2Vec2 vel(b->GetLinearVelocity());
		float32 speed = vel.Normalize();
		float32 angular_speed = b->GetAngularVelocity();

		//if (physics.air_resistance > 0.f) {
		//	auto force_dir = physics.get_mass() * -vel;
		//	auto force_components = (physics.air_resistance  * speed * speed);
		//
		//	//if (speed > 1.0)
		//	//	force_components += (0.5f * sqrt(std::abs(speed)));
		//	
		//	physics.body->ApplyForce(force_components * force_dir, physics.body->GetWorldCenter(), true);
		//}

		auto angular_resistance = physics.angular_air_resistance;
		if (angular_resistance < 0.f) angular_resistance = physics.air_resistance;
		
		if (angular_resistance > 0.f) {
			//physics.body->ApplyTorque((angular_resistance * sqrt(sqrt(angular_speed * angular_speed)) + 0.2 * angular_speed * angular_speed)* -sgn(angular_speed) * b->GetInertia(), true);
			physics.body->ApplyTorque((angular_resistance * angular_speed * angular_speed )* -sgn(angular_speed) * b->GetInertia(), true);
		}

		if (physics.enable_angle_motor) {
			float nextAngle = static_cast<float>(b->GetAngle() + b->GetAngularVelocity() / parent_overworld.delta_timer.get_steps_per_second());
			float totalRotation = (constrainAngle(physics.target_angle) * DEG_TO_RAD) - nextAngle;

			totalRotation *= RAD_TO_DEG;

			totalRotation = constrainAngle(totalRotation);
			//if (totalRotation > 180) {
			//	totalRotation = std::fmod(totalRotation + 180, 360) - 180;
			//}
			//
			//if (totalRotation < -180) {
			//	totalRotation = std::fmod(-(totalRotation + 180), 360) - 180;
			//}

			totalRotation *= DEG_TO_RAD;

			float desiredAngularVelocity = totalRotation / static_cast<float>(parent_overworld.delta_timer.delta_seconds());
			float impulse = b->GetInertia() * desiredAngularVelocity;// disregard time factor
			b->ApplyAngularImpulse(impulse*physics.angle_motor_force_multiplier, true);
		}
	}

	listener.after_step_callbacks.clear();

	b2world.Step(static_cast<float32>(delta_seconds()), velocityIterations, positionIterations);
	b2world.ClearForces();

	for (auto& c : listener.after_step_callbacks)
		c();

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto entity = b->GetUserData();
		auto& physics = entity->get<components::physics>();
		
		recurential_friction_handler(b->GetUserData(), physics.get_owner_friction_ground());
	}

	reset_states();
}

void physics_system::rechoose_owner_friction_body(augs::entity_id entity) {
	auto& physics = entity->get<components::physics>();
	
	// purge of dead entities

	physics.owner_friction_grounds.erase(std::remove_if(physics.owner_friction_grounds.begin(), physics.owner_friction_grounds.end(), [entity](entity_id subject) {
		return subject.dead();
	}), physics.owner_friction_grounds.end());

	auto feasible_grounds = physics.owner_friction_grounds;

	if (!feasible_grounds.empty()) {
		// cycle guard
		// remove friction grounds whom do I own myself

		feasible_grounds.erase(std::remove_if(feasible_grounds.begin(), feasible_grounds.end(), [entity](entity_id subject) {
			return components::physics::are_connected_by_friction(subject, entity);
		}), feasible_grounds.end());
	}

	if (!feasible_grounds.empty()) {
		std::stable_sort(feasible_grounds.begin(), feasible_grounds.end(), [](entity_id a, entity_id b) {
			return components::physics::are_connected_by_friction(a, b);
		});

		physics.owner_friction_ground = feasible_grounds[0];

		// make the new owner first in order in case it is later compared to the same ancestor-level parents

		for (auto& it = physics.owner_friction_grounds.begin(); it != physics.owner_friction_grounds.end(); ++it) {
			if (*it == physics.owner_friction_ground) {
				std::swap(physics.owner_friction_grounds[0], *it);
			}
		}

		/// consider friction grounds ONLY from the same ancestor line, and only the descendants

		/// if the current one is not found within contacting friction grounds,
		/// prioritize like this:
		/// firstly, the lowest descendant of the ancestor line of the lost friction ground
		/// descendant of any other tree with the biggest height, stable-sorted in order of entrance

	}
	else {
		physics.owner_friction_ground.unset();
	}
}

void physics_system::recurential_friction_handler(entity_id entity, entity_id friction_owner) {
	if (friction_owner.dead()) return;

	auto& physics = entity->get<components::physics>();
	
	auto& friction_physics = friction_owner->get<components::fixtures>();
	auto& friction_entity = friction_physics.get_body_entity();

	recurential_friction_handler(entity, friction_entity->get<components::physics>().get_owner_friction_ground());

	auto friction_body = friction_physics.get_body();
	auto fricted_pos = physics.body->GetPosition() + delta_seconds() * friction_body->GetLinearVelocityFromWorldPoint(physics.body->GetPosition());

	physics.body->SetTransform(fricted_pos, physics.body->GetAngle() + delta_seconds()*friction_body->GetAngularVelocity());

	friction_entity->get<components::physics>().measured_carried_mass += physics.get_mass() + physics.measured_carried_mass;
}

void physics_system::destroy_whole_world() {
	std::vector<b2Body*> to_destroy;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext())
		to_destroy.push_back(b);

	for (auto& just_die : to_destroy)
		b2world.DestroyBody(just_die);
}


#include "../components/render_component.h"
void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity_id>(b->GetUserData())->get<components::transform>();
		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();

		auto body_pos = METERS_TO_PIXELSf * b->GetPosition();
		auto body_angle = b->GetAngle() * RAD_TO_DEG;

		for (auto& fe : physics.fixture_entities) {
			auto& fixtures = fe->get<components::fixtures>();

			auto& transform = fe->get<components::transform>();
			transform.pos = body_pos;

			if (!b->IsFixedRotation())
				transform.rotation = body_angle;

			transform.pos += fixtures.shape_offset.pos;
			transform.rotation += fixtures.shape_offset.rotation;

			transform.pos.rotate(body_angle, body_pos);
		}

		transform.pos = body_pos;

		if (!b->IsFixedRotation())
			transform.rotation = body_angle;
	}
}

void physics_system::create_physics_for_entity(augs::entity_id e) {
	/* remove dead components that are remnants of cloning or mistakenly added */
	e->remove<components::physics>();
	e->remove<components::fixtures>();

	auto* physics_definition = e->find<components::physics_definition>();

	if (physics_definition) {
		if (!physics_definition->dont_create_fixtures_and_body) {
			auto other_body_entity = physics_definition->attach_fixtures_to_entity;

			if (other_body_entity.dead() || other_body_entity == e)
				create_physics_component(physics_definition->body, e);

			for (auto fixture : physics_definition->fixtures) {
				fixture.transform_vertices += physics_definition->offset_created_shapes;

				if (other_body_entity.alive())
					add_fixtures_to_other_body(fixture, e, physics_definition->attach_fixtures_to_entity);
				else
					add_fixtures(fixture, e);
			}
		}

		if (!physics_definition->preserve_definition_for_cloning)
			e->remove<components::physics_definition>();
	}
}

void physics_system::destroy_physics_of_entity(augs::entity_id subject) {
	auto* maybe_physics = subject->find<components::physics>();

	if (maybe_physics) {
		b2world.DestroyBody(maybe_physics->body);

		for (auto fe : maybe_physics->fixture_entities) {
			fe->remove<components::fixtures>();
		}

		subject->remove<components::physics>();
	}

	auto* maybe_fixtures = subject->find<components::fixtures>();

	if (maybe_fixtures) {
		auto& fixture_list = maybe_fixtures->get_body_entity()->get<components::physics>().fixture_entities;
		fixture_list.erase(std::remove(fixture_list.begin(), fixture_list.end(), subject), fixture_list.end());

		for (auto f : maybe_fixtures->list_of_fixtures)
			f.fixture->GetBody()->DestroyFixture(f.fixture);

		subject->remove<components::fixtures>();
	}
}

void physics_system::create_bodies_and_fixtures_from_physics_definitions() {
	auto& events = parent_world.get_message_queue<messages::new_entity_message>();

	for (auto& it : events)
		create_physics_for_entity(it.subject);
}

void physics_system::destroy_fixtures_and_bodies() {
	auto& to_destroy = parent_world.get_message_queue<messages::destroy_message>();

	for (auto& m : to_destroy)
		destroy_physics_of_entity(m.subject);
}
