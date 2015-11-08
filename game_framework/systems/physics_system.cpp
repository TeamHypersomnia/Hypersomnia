#include "physics_system.h"
double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/collision_message.h"
#include "../game/body_helper.h"

#include <iostream>


physics_system::stepped_timer::stepped_timer(physics_system* owner) : owner(owner), current_step(-1) {
	reset();
}

void physics_system::stepped_timer::reset() {
	current_step = owner->all_steps;
}

float physics_system::stepped_timer::get_milliseconds() const {
	return get_steps() * static_cast<float>(owner->accumulator.get_timestep());
}

float physics_system::stepped_timer::extract_milliseconds() {
	float result = get_milliseconds();
	reset();
	return result;
}

unsigned physics_system::stepped_timer::get_steps() const {
	return owner->all_steps - current_step;
}

unsigned physics_system::stepped_timer::extract_steps() {
	unsigned result = get_steps();
	reset();
	return result;
}

bool physics_system::raycast_input::ShouldRaycast(b2Fixture* fixture) {
	entity_id fixture_entity = fixture->GetBody()->GetUserData();
	return
		(!subject || fixture_entity != subject) &&
		(b2ContactFilter::ShouldCollide(subject_filter, &fixture->GetFilterData()));
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

physics_system::physics_system() : accumulator(60.0, 5), timestep_multiplier(1.f),
b2world(b2Vec2(0.f, 0.f)), enable_interpolation(true), ray_casts_per_frame(0), all_steps(0) {
		b2world.SetAllowSleeping(false);
		b2world.SetAutoClearForces(false);
		enable_listener(true);
}

std::vector<physics_system::raycast_output> physics_system::ray_cast_all_intersections
	(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity) {
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
		auto out = ray_cast_px(position, position + vec2::from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

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
		auto out = ray_cast_px(position, position + vec2::from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

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
	callback.subject_filter = &filter;
	callback.subject = ignore_entity;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		printf("ray_cast: X: %f, Y: %f\nX: %f, Y: %f\n", p1_meters.x, p1_meters.y, p2_meters.x, p2_meters.y);
		std::cout << "error" << std::endl;

		return callback.output;
	}

	b2world.RayCast(&callback, p1_meters, p2_meters);
	return callback.output;
}

physics_system::raycast_output physics_system::ray_cast_px (vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity) {
	auto out = ray_cast(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
	out.intersection *= METERS_TO_PIXELSf;
	
	return out;
}

bool physics_system::query_aabb_input::ReportFixture(b2Fixture* fixture) {
	if ((b2ContactFilter::ShouldCollide(filter, &fixture->GetFilterData()))
		&& fixture->GetBody()->GetUserData() != ignore_entity) {
		output.insert(fixture->GetBody());
		out_fixtures.push_back(fixture);
	}

	return true;
}

physics_system::query_output physics_system::query_square(vec2 p1_meters, float side_meters, b2Filter* filter, entity_id ignore_entity) {
	b2AABB aabb;
	aabb.lowerBound = p1_meters - side_meters / 2;
	aabb.upperBound = p1_meters + side_meters / 2;
	return query_aabb(aabb.lowerBound, aabb.upperBound, filter, ignore_entity);
}

physics_system::query_output physics_system::query_square_px(vec2 p1, float side, b2Filter* filter, entity_id ignore_entity) {
	return query_square(p1 * PIXELS_TO_METERSf, side * PIXELS_TO_METERSf, filter, ignore_entity);
}

physics_system::query_output physics_system::query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter* filter, entity_id ignore_entity) {
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;
	b2AABB aabb;
	aabb.lowerBound = p1_meters;
	aabb.upperBound = p2_meters;

	b2world.QueryAABB(&callback, aabb);

	physics_system::query_output out;
	out.bodies = std::move(std::vector<b2Body*>(callback.output.begin(), callback.output.end()));
	return out;
}

physics_system::query_output physics_system::query_body(augs::entity_id subject, b2Filter* filter, entity_id ignore_entity) {
	query_output total_output;
	
	for (b2Fixture* f = subject->get<components::physics>().body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
		auto transformed = helpers::get_transformed_shape_verts(subject, true);
		
		b2PolygonShape shape;
		shape.Set(transformed.data(), transformed.size());

		auto this_result = query_shape(&shape, filter, ignore_entity);
		total_output.bodies.insert(total_output.bodies.end(), this_result.bodies.begin(), this_result.bodies.end());
	}

	return total_output;
}

physics_system::query_output physics_system::query_polygon(const std::vector<vec2>& vertices, b2Filter* filter, entity_id ignore_entity) {
	b2PolygonShape poly_shape;
	std::vector<b2Vec2> verts;

	for (auto& v : vertices)
		verts.push_back(PIXELS_TO_METERSf * b2Vec2(v.x, v.y));
	
	poly_shape.Set(verts.data(), verts.size());
	return query_shape(&poly_shape, filter, ignore_entity);
}

physics_system::query_output physics_system::query_shape(b2Shape* shape, b2Filter* filter, entity_id ignore_entity) {
	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
	
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;
	
	b2AABB shape_aabb;
	shape->ComputeAABB(&shape_aabb, null_transform, 0);
	b2world.QueryAABB(&callback, shape_aabb);
	
	std::set<b2Body*> bodies;
	std::set<physics_system::query_output::queried_result> details;

	for (auto fixture : callback.out_fixtures) {
		auto result = b2TestOverlapInfo(shape, 0, fixture->GetShape(), 0, null_transform, fixture->GetBody()->GetTransform());
		if (result.overlap) {
			bodies.insert(fixture->GetBody());
			details.insert({
				fixture->GetBody(),
				result.pointA
			});
		}
	}

	physics_system::query_output out;
	out.bodies = std::vector<b2Body*>(bodies.begin(), bodies.end());
	out.details = std::vector<physics_system::query_output::queried_result>(details.begin(), details.end());

	return out;
}

physics_system::query_output physics_system::query_aabb_px(vec2 p1, vec2 p2, b2Filter* filter, entity_id ignore_entity) {
	return query_aabb(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
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

		msg.subject = static_cast<entity_id>(body_a->GetUserData());
		msg.collider = static_cast<entity_id>(body_b->GetUserData());

		if (fix_a->IsSensor()) {
			msg.subject_impact_velocity = (body_a->GetLinearVelocity());
			msg.collider_impact_velocity = (body_b->GetLinearVelocity());
			world_ptr->post_message(msg);
		}

		if (fix_b->IsSensor()) {
			msg.subject_impact_velocity = (body_b->GetLinearVelocity());
			msg.collider_impact_velocity = (body_a->GetLinearVelocity());

			std::swap(msg.collider, msg.subject);
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

		msg.subject = static_cast<entity_id>(body_a->GetUserData());
		msg.collider = static_cast<entity_id>(body_b->GetUserData());

		msg.sensor_end_contact = true;
		
		if (fix_a->IsSensor()) {
			msg.subject_impact_velocity = -body_a->GetLinearVelocity();
			msg.collider_impact_velocity = -body_b->GetLinearVelocity();
			world_ptr->post_message(msg);
		}

		if (fix_b->IsSensor()) {
			std::swap(msg.collider, msg.subject);
			msg.subject_impact_velocity = -body_b->GetLinearVelocity();
			msg.collider_impact_velocity = -body_a->GetLinearVelocity();
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
T constrainAngle(T x){
	x = static_cast<T>(fmod(x + 180, 360));
	if (x < 0)
		x += 360;
	return x - 180;
}

void physics_system::enable_listener(bool flag) {
	b2world.SetContactListener(flag ? &listener : nullptr);
}


void physics_system::process_steps(world& owner, unsigned steps) {
	listener.world_ptr = &owner;

	for (unsigned i = 0; i < steps; ++i) {
		int32 velocityIterations = 8;
		int32 positionIterations = 3;

		if (enable_interpolation)
			reset_states();

		if (enable_motors)
			for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
				if (b->GetType() == b2_staticBody) continue;
				auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();

				if (physics.enable_angle_motor) {
					float nextAngle = static_cast<float>(b->GetAngle() + b->GetAngularVelocity() / accumulator.get_hz());
					float totalRotation = (constrainAngle(physics.target_angle) * 0.01745329251994329576923690768489f) - nextAngle;
					while (totalRotation < -180 * 0.01745329251994329576923690768489f) totalRotation += 360 * 0.01745329251994329576923690768489f;
					while (totalRotation >  180 * 0.01745329251994329576923690768489f) totalRotation -= 360 * 0.01745329251994329576923690768489f;
					float desiredAngularVelocity = totalRotation * static_cast<float>(accumulator.get_hz());
					float impulse = b->GetInertia() * desiredAngularVelocity;// disregard time factor
					b->ApplyAngularImpulse(impulse*physics.angle_motor_force_multiplier, true);
				}
			}

		owner.get_message_queue<messages::collision_message>().clear();

		if (prestepping_routine)
			prestepping_routine(owner);

		b2world.Step(static_cast<float32>(accumulator.per_second()), velocityIterations, positionIterations);
		b2world.ClearForces();
		++all_steps;

		if (poststepping_routine)
			poststepping_routine(owner);
	}

	if (steps == 0) b2world.ClearForces();

	if (enable_interpolation)
		smooth_states();
	else reset_states();
}

unsigned physics_system::process_entities(world& owner) {
	accumulator.set_time_multiplier(timestep_multiplier);
	const unsigned steps = accumulator.update_and_extract_steps();

	process_steps(owner, steps);

	return steps;
}

void physics_system::add(entity_id) {

}

void physics_system::clear() {
	std::vector<b2Body*> to_destroy;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) 
		to_destroy.push_back(b);

	for (auto& just_die : to_destroy)
		b2world.DestroyBody(just_die);

	processing_system::clear();
}

void physics_system::configure_stepping(float fps, int max_updates_per_step) {
	accumulator = augs::delta_accumulator(fps, max_updates_per_step);
}

double physics_system::get_timestep_ms() {
	return accumulator.get_timestep();
}

void physics_system::remove(entity_id e) {
	b2world.DestroyBody(e->get<components::physics>().body);
}

void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity_id>(b->GetUserData())->get<components::transform>();
		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();
		
		transform.current.pos = b->GetPosition();
		transform.current.pos *= METERS_TO_PIXELSf;
		
		if (!b->IsFixedRotation()) {
			transform.current.rotation = b->GetAngle();
			transform.current.rotation *= 180.0f / 3.141592653589793238462f;
		}
		
		transform.previous = transform.current;
	}
}

void physics_system::smooth_states() {
	const float ratio = static_cast<float>(accumulator.get_ratio());
 
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
 
		auto& transform = static_cast<entity_id>(b->GetUserData())->get<components::transform>();

		transform.current.pos = transform.previous.pos + ratio * (METERS_TO_PIXELSf*b->GetPosition() - transform.previous.pos);
		
		if (!b->IsFixedRotation())
			transform.current.rotation = static_cast<float>(transform.previous.rotation + ratio * (b->GetAngle()*180.0/3.141592653589793238462 - transform.previous.rotation));
	}
}