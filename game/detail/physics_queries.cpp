#include "game/systems_temporary/physics_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "augs/ensure.h"
#include "game/detail/physics_scripts.h"

bool physics_system::raycast_input::ShouldRaycast(b2Fixture* const fixture) {
	const auto fixture_entity = fixture->GetBody()->GetUserData();
	return
		(subject == entity_id() || fixture_entity != subject) &&
		(b2ContactFilter::ShouldCollide(&subject_filter, &fixture->GetFilterData()));
}

float32 physics_system::raycast_input::ReportFixture(b2Fixture* const fixture, const b2Vec2& point,
	const b2Vec2& normal, const float32 fraction) {
	output.intersection = point;

	output.hit = true;
	output.what_entity = fixture->GetBody()->GetUserData();
	output.normal = normal;

	if (save_all) {
		outputs.push_back(output);
		return 1.f;
	}

	return fraction;
}

std::vector<physics_system::raycast_output> physics_system::ray_cast_all_intersections
(const vec2 p1_meters, const vec2 p2_meters, const b2Filter filter, const entity_id ignore_entity) const {
	++ray_casts_since_last_step;

	raycast_input callback;
	callback.subject = ignore_entity;
	callback.subject_filter = filter;
	callback.save_all = true;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
	}

	b2world->RayCast(&callback, p1_meters, p2_meters);
	return callback.outputs;
}

physics_system::edge_edge_output physics_system::edge_edge_intersection(const vec2 p1_meters, const vec2 p2_meters, const vec2 edge_p1, const vec2 edge_p2) {
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

float physics_system::get_closest_wall_intersection(const vec2 position, const float radius, const int ray_amount, const b2Filter filter, const entity_id ignore_entity) const {
	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		const auto out = ray_cast_px(position, position + vec2().set_from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
		}
	}

	return worst_distance;
}

vec2 physics_system::push_away_from_walls(const vec2 position, const float radius, const int ray_amount, const b2Filter filter, const entity_id ignore_entity) const {
	vec2 resultant;

	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		const auto out = ray_cast_px(position, position + vec2().set_from_degrees((360.f / ray_amount) * i) * radius, filter, ignore_entity);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
			resultant += diff;
		}
	}

	if (resultant.non_zero()) {
		return position + (-resultant).set_length(radius - worst_distance);
	}
	else {
		return position;
	}
}

physics_system::raycast_output physics_system::ray_cast(const vec2 p1_meters, const vec2 p2_meters, const b2Filter filter, const entity_id ignore_entity) const {
	++ray_casts_since_last_step;

	raycast_input callback;
	callback.subject = ignore_entity;
	callback.subject_filter = filter;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
		return callback.output;
	}

	b2world->RayCast(&callback, p1_meters, p2_meters);
	return callback.output;
}

physics_system::raycast_output physics_system::ray_cast_px(const vec2 p1, const vec2 p2, const b2Filter filter, const entity_id ignore_entity) const {
	auto out = ray_cast(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
	out.intersection *= METERS_TO_PIXELSf;

	return out;
}

bool physics_system::query_aabb_input::ReportFixture(b2Fixture* const fixture) {
	if ((b2ContactFilter::ShouldCollide(&filter, &fixture->GetFilterData()))
		&& fixture->GetBody()->GetUserData() != ignore_entity) {
		out.bodies.insert(fixture->GetBody());
		out.fixtures.push_back(fixture);
		out.entities.insert(fixture->GetUserData());
	}

	return true;
}

physics_system::query_aabb_output physics_system::query_square(const vec2 p1_meters, const float side_meters, const b2Filter filter, const entity_id ignore_entity) const {
	b2AABB aabb;
	aabb.lowerBound = p1_meters - side_meters / 2;
	aabb.upperBound = p1_meters + side_meters / 2;
	return query_aabb(aabb.lowerBound, aabb.upperBound, filter, ignore_entity);
}

physics_system::query_aabb_output physics_system::query_square_px(const vec2 p1, const float side, const b2Filter filter, const entity_id ignore_entity) const {
	return query_square(p1 * PIXELS_TO_METERSf, side * PIXELS_TO_METERSf, filter, ignore_entity);
}

physics_system::query_aabb_output physics_system::query_aabb(const vec2 p1_meters, const vec2 p2_meters, const b2Filter filter, const entity_id ignore_entity) const {
	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;
	b2AABB aabb;
	aabb.lowerBound = p1_meters;
	aabb.upperBound = p2_meters;

	b2world->QueryAABB(&callback, aabb);

	return callback.out;
}

physics_system::query_output physics_system::query_body(const const_entity_handle subject, const b2Filter filter, const entity_id ignore_entity) const {
	query_output total_output;

	const auto cache = get_rigid_body_cache(subject);

	for (const b2Fixture* f = cache.body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
		auto world_vertices = get_world_vertices(subject, true);

		b2PolygonShape shape;
		shape.Set(world_vertices.data(), world_vertices.size());

		total_output += query_shape(&shape, filter, ignore_entity);
	}

	return total_output;
}

physics_system::query_output physics_system::query_polygon(const std::vector<vec2>& vertices, const b2Filter filter, const entity_id ignore_entity) const {
	b2PolygonShape poly_shape;
	std::vector<b2Vec2> verts;

	for (const auto v : vertices) {
		verts.push_back(PIXELS_TO_METERSf * b2Vec2(v.x, v.y));
	}

	poly_shape.Set(verts.data(), verts.size());
	return query_shape(&poly_shape, filter, ignore_entity);
}

physics_system::query_output physics_system::query_shape(const b2Shape* const shape, const b2Filter filter, const entity_id ignore_entity) const {
	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

	query_aabb_input callback;
	callback.filter = filter;
	callback.ignore_entity = ignore_entity;

	b2AABB shape_aabb;
	shape->ComputeAABB(&shape_aabb, null_transform, 0);
	b2world->QueryAABB(&callback, shape_aabb);

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

physics_system::query_aabb_output physics_system::query_aabb_px(const vec2 p1, const vec2 p2, const b2Filter filter, const entity_id ignore_entity) const {
	return query_aabb(p1 * PIXELS_TO_METERSf, p2 * PIXELS_TO_METERSf, filter, ignore_entity);
}