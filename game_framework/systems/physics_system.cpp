#include "physics_system.h"
double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../game/body_helper.h"
#include "utilities/print.h"

#include "../components/fixtures_component.h"

#include <iostream>

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
		printf("ray_cast: X: %f, Y: %f\nX: %f, Y: %f\n", p1_meters.x, p1_meters.y, p2_meters.x, p2_meters.y);
		std::cout << "error" << std::endl;
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
		printf("ray_cast: X: %f, Y: %f\nX: %f, Y: %f\n", p1_meters.x, p1_meters.y, p2_meters.x, p2_meters.y);
		std::cout << "error" << std::endl;

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
		auto world_vertices = helpers::get_world_vertices(subject, true);

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

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	for (int i = 0; i < 2; ++i) {
		auto fix_a = contact->GetFixtureA();
		auto fix_b = contact->GetFixtureB();

		if (i == 1)
			std::swap(fix_a, fix_b);

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
			if (
				//!components::physics::are_connected_by_friction(msg.collider, msg.subject) &&
				!components::physics::are_connected_by_friction(msg.subject, msg.collider) &&
				!collider_fixtures.is_friction_ground)
			{
				auto& collider_physics = collider_fixtures.get_body_entity()->get<components::physics>();
				collider_physics.owner_friction_grounds.push_back(subject_fixtures.get_body_entity());
			}
		}

		if (fix_a->IsSensor()) {
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
			for (auto it = collider_physics.owner_friction_grounds.begin(); it != collider_physics.owner_friction_grounds.end(); ++it)
				if (*it == subject_fixtures.get_body_entity())
				{
					// TODO: see if the new friction field fulfills the same constraints as in BeginContact

					collider_physics.owner_friction_grounds.erase(it);
					break;
				}
		}

		if (fix_a->IsSensor()) {
			msg.sensor_end_contact = true;
			msg.subject_impact_velocity = -body_a->GetLinearVelocity();
			msg.collider_impact_velocity = -body_b->GetLinearVelocity();
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

	msg.subject = static_cast<entity_id>(body_a->GetUserData());
	msg.collider = static_cast<entity_id>(body_b->GetUserData());

	msg.point = manifold.points[0];
	msg.point *= METERS_TO_PIXELSf;

	msg.subject_impact_velocity = body_a->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	msg.collider_impact_velocity = body_b->GetLinearVelocityFromWorldPoint(manifold.points[0]);
	world_ptr->post_message(msg);

	std::swap(msg.collider, msg.subject);
	std::swap(msg.subject_impact_velocity, msg.collider_impact_velocity);
	world_ptr->post_message(msg);
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

		// auto angular_resistance = physics.angular_air_resistance;
		// if (angular_resistance < 0.f) angular_resistance = physics.air_resistance;
		// 
		// if (angular_resistance > 0.f) {
		// 	//physics.body->ApplyTorque((angular_resistance * sqrt(sqrt(angular_speed * angular_speed)) + 0.2 * angular_speed * angular_speed)* -sgn(angular_speed) * b->GetInertia(), true);
		// 	physics.body->ApplyTorque(( 1.2 * angular_speed * angular_speed )* -sgn(angular_speed) * b->GetInertia(), true);
		// }

		if (physics.enable_angle_motor) {
			float nextAngle = static_cast<float>(b->GetAngle() + b->GetAngularVelocity() / parent_overworld.accumulator.get_hz());
			float totalRotation = (constrainAngle(physics.target_angle) * 0.01745329251994329576923690768489f) - nextAngle;

			totalRotation /= 0.01745329251994329576923690768489f;

			totalRotation = constrainAngle(totalRotation);
			//if (totalRotation > 180) {
			//	totalRotation = std::fmod(totalRotation + 180, 360) - 180;
			//}
			//
			//if (totalRotation < -180) {
			//	totalRotation = std::fmod(-(totalRotation + 180), 360) - 180;
			//}

			totalRotation *= 0.01745329251994329576923690768489f;

			float desiredAngularVelocity = totalRotation * static_cast<float>(parent_overworld.accumulator.get_hz());
			float impulse = b->GetInertia() * desiredAngularVelocity;// disregard time factor
			b->ApplyAngularImpulse(impulse*physics.angle_motor_force_multiplier, true);
		}
	}

	listener.after_step_callbacks.clear();

	b2world.Step(static_cast<float32>(per_second()), velocityIterations, positionIterations);
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

void physics_system::recurential_friction_handler(entity_id entity, entity_id friction_owner) {
	if (friction_owner.dead()) return;

	auto& physics = entity->get<components::physics>();
	
	auto& friction_physics = friction_owner->get<components::fixtures>();
	auto& friction_entity = friction_physics.get_body_entity();

	recurential_friction_handler(entity, friction_entity->get<components::physics>().get_owner_friction_ground());

	auto friction_body = friction_physics.get_body();

	auto friction_vel = friction_body->GetLinearVelocity();
	auto friction_ang_vel = friction_body->GetAngularVelocity();
	auto friction_center = friction_body->GetWorldCenter();

	auto fricted_pos = physics.body->GetPosition() + per_second() * friction_vel;

	auto rotation_offset = friction_center - fricted_pos;
	b2Vec2 rotational_velocity = (-vec2(rotation_offset).perpendicular_cw()).set_length(
		rotation_offset.Length()*friction_ang_vel
		);
	//rotational_velocity.y *= -1;

	fricted_pos += per_second() * rotational_velocity;
	//renderer::get_current().logic_lines.draw_cyan(physics.get_position(), physics.get_position() + METERS_TO_PIXELSf*friction_vel);

	physics.body->SetTransform(fricted_pos,
		physics.body->GetAngle()
		+ per_second()*friction_ang_vel
		);

	friction_entity->get<components::physics>().measured_carried_mass += physics.get_mass() + physics.measured_carried_mass;
}

void physics_system::destroy_whole_world() {
	std::vector<b2Body*> to_destroy;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext())
		to_destroy.push_back(b);

	for (auto& just_die : to_destroy)
		b2world.DestroyBody(just_die);
}

void physics_system::destroy_fixtures_and_bodies() {
	auto& to_destroy = parent_world.get_message_queue<messages::destroy_message>();

	for (auto& m : to_destroy) {
		auto* maybe_physics = m.subject->find<components::physics>();

		if (maybe_physics) {
			b2world.DestroyBody(maybe_physics->body);

			for (auto fe : maybe_physics->fixture_entities) {
				parent_world.post_message(messages::destroy_message(fe));
			}
		}
	}

	for (auto& m : to_destroy) {
		auto* maybe_fixtures = m.subject->find<components::fixtures>();

		if (maybe_fixtures) {
			for (auto f : maybe_fixtures->list_of_fixtures) {
				f.fixture->GetBody()->DestroyFixture(f.fixture);
			}
		}
	}
}

#include "../components/render_component.h"
void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity_id>(b->GetUserData())->get<components::transform>();
		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();

		auto body_pos = METERS_TO_PIXELSf * b->GetPosition();
		auto body_angle = b->GetAngle() * 180.0f / 3.141592653589793238462f;

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