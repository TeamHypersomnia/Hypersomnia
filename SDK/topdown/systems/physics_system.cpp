#include "stdafx.h"
#include "physics_system.h"
#include "entity_system/entity.h"

#include "../components/damage_component.h"

#include "../messages/collision_message.h"

#include "../game/body_helper.h"

bool physics_system::raycast_input::ShouldRaycast(b2Fixture* fixture) {
	entity* fixture_entity = reinterpret_cast<entity*>(fixture->GetBody()->GetUserData());
	return
		(!subject || fixture_entity != subject) &&
		(b2ContactFilter::ShouldCollide(subject_filter, &fixture->GetFilterData()));
}

float32 physics_system::raycast_input::ReportFixture(b2Fixture* fixture, const b2Vec2& point,
	const b2Vec2& normal, float32 fraction) {
		output.intersection = point;

		output.hit = true;
		output.what_fixture = fixture;
		output.what_entity = reinterpret_cast<entity*>(fixture->GetBody()->GetUserData());
		output.normal = normal;

		if (save_all) {
			outputs.push_back(output);
			return 1.f;
		}
		
		return fraction;
}

physics_system::raycast_input::raycast_input() : subject_filter(nullptr), subject(nullptr), save_all(false) {}

physics_system::physics_system() : accumulator(60.0, 5), timestep_multiplier(1.f),
b2world(b2Vec2(0.f, 0.f)), enable_interpolation(true), ray_casts_per_frame(0) {
		b2world.SetAllowSleeping(false);
		b2world.SetAutoClearForces(false);
		b2world.SetContactListener(&listener);
}

std::vector<physics_system::raycast_output> physics_system::ray_cast_all_intersections
	(vec2<> p1_meters, vec2<> p2_meters, b2Filter filter, entity* ignore_entity) {
	++ray_casts_per_frame;

	raycast_input callback;
	callback.subject_filter = &filter;
	callback.subject = ignore_entity;
	callback.save_all = true;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		printf("ray_cast: X: %f, Y: %f\nX: %f, Y: %f\n", p1_meters.x, p1_meters.y, p2_meters.x, p2_meters.y);
		std::cout << "error" << std::endl;
	}

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.outputs;
}

physics_system::edge_edge_output physics_system::edge_edge_intersection(vec2<> p1_meters, vec2<> p2_meters, vec2<> edge_p1, vec2<> edge_p2) {
	/* prepare b2RayCastOutput/b2RayCastInput data for raw b2EdgeShape::RayCast call */
	b2RayCastOutput output;
	b2RayCastInput input;
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

vec2<> physics_system::push_away_from_walls(vec2<> position, float radius, int ray_amount, b2Filter filter, entity* ignore_entity) {
	vec2<> resultant;
	
	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		auto out = ray_cast_px(position, position + vec2<>::from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

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

physics_system::raycast_output physics_system::ray_cast(vec2<> p1_meters, vec2<> p2_meters, b2Filter filter, entity* ignore_entity) {
	++ray_casts_per_frame;

	raycast_input callback;
	callback.subject_filter = &filter;
	callback.subject = ignore_entity;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		printf("ray_cast: X: %f, Y: %f\nX: %f, Y: %f\n", p1_meters.x, p1_meters.y, p2_meters.x, p2_meters.y);
		std::cout << "error" << std::endl;
	}

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.output;
}

physics_system::raycast_output physics_system::ray_cast_px (vec2<> p1, vec2<> p2, b2Filter filter, entity* ignore_entity) {
	auto out = ray_cast(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
	out.intersection *= METERS_TO_PIXELSf;
	
	return out;
}

physics_system::query_aabb_input::query_aabb_input() : ignore_userdata(nullptr), filter(nullptr) {}

bool physics_system::query_aabb_input::ReportFixture(b2Fixture* fixture) {
	if ((b2ContactFilter::ShouldCollide(filter, &fixture->GetFilterData()))
		&& fixture->GetBody()->GetUserData() != ignore_userdata) {
		output.insert(fixture->GetBody());
		out_fixtures.push_back(fixture);
	}

	return true;
}

physics_system::query_output physics_system::query_square(vec2<> p1_meters, float side_meters, b2Filter* filter, void* ignore_userdata) {
	b2AABB aabb;
	aabb.lowerBound = p1_meters - side_meters / 2;
	aabb.upperBound = p1_meters + side_meters / 2;
	return query_aabb(aabb.lowerBound, aabb.upperBound, filter, ignore_userdata);
}

physics_system::query_output physics_system::query_square_px(vec2<> p1, float side, b2Filter* filter, void* ignore_userdata) {
	return query_square(p1 * PIXELS_TO_METERSf, side * PIXELS_TO_METERSf, filter, ignore_userdata);
}

physics_system::query_output physics_system::query_aabb(vec2<> p1_meters, vec2<> p2_meters, b2Filter* filter, void* ignore_userdata) {
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_userdata = ignore_userdata;
	b2AABB aabb;
	aabb.lowerBound = p1_meters;
	aabb.upperBound = p2_meters;

	b2world.QueryAABB(&callback, aabb);
	return std::move(std::vector<b2Body*>(callback.output.begin(), callback.output.end()));
}

physics_system::query_output physics_system::query_body(augmentations::entity_system::entity& subject, b2Filter* filter, void* ignore_userdata) {
	query_output total_output;
	
	for (b2Fixture* f = subject.get<components::physics>().body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
		auto transformed = topdown::get_transformed_shape_verts(subject, true);
		
		b2PolygonShape shape;
		shape.Set(transformed.data(), transformed.size());

		auto this_result = query_shape(&shape, filter, ignore_userdata);
		total_output.bodies.insert(total_output.bodies.end(), this_result.bodies.begin(), this_result.bodies.end());
	}

	return total_output;
}

physics_system::query_output physics_system::query_shape(b2Shape* shape, b2Filter* filter, void* ignore_userdata) {
	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
	
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_userdata = ignore_userdata;
	
	b2AABB shape_aabb;
	shape->ComputeAABB(&shape_aabb, null_transform, 0);
	b2world.QueryAABB(&callback, shape_aabb);
	
	std::set<b2Body*> bodies;

	for (auto fixture : callback.out_fixtures) 
		if (b2TestOverlap(shape, 0, fixture->GetShape(), 0, null_transform, fixture->GetBody()->GetTransform())) 
			bodies.insert(fixture->GetBody());

	return std::move(std::vector<b2Body*>(bodies.begin(), bodies.end()));
}

physics_system::query_output physics_system::query_aabb_px(vec2<> p1, vec2<> p2, b2Filter* filter, void* ignore_userdata) {
	return query_aabb(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_userdata);
}

void physics_system::contact_listener::BeginContact(b2Contact* contact) {
	auto fix_a = contact->GetFixtureA();
	auto fix_b = contact->GetFixtureB();

	/* collision messaging happens only for sensors here
		PreSolve is the counterpart for regular bodies
	*/
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
	accumulator.set_time_multiplier(timestep_multiplier);
	const unsigned steps = accumulator.update_and_extract_steps();

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		reset_states();
		
		substepping_routine(owner);
		//for (auto& sys : subsystems)
		//	sys->substep(owner);

		b2world.Step(static_cast<float32>(accumulator.per_second()), velocityIterations, positionIterations);
		b2world.ClearForces();
	}
	
	if(steps == 0) b2world.ClearForces();
	
	if (enable_interpolation)
		smooth_states();

	ray_casts_per_frame = 0;
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