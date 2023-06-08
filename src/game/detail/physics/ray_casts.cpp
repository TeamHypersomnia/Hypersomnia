#include "game/inferred_caches/physics_world_cache.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/enums/filters.h"

struct raycast_input : public b2RayCastCallback {
	entity_id subject;
	b2Filter subject_filter;

	bool save_all = false;
	physics_raycast_output output;
	std::vector<physics_raycast_output> outputs;

	bool ShouldRaycast(b2Fixture* fixture) override;
	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override;
};

bool raycast_input::ShouldRaycast(b2Fixture* const fixture) {
	const auto fixture_entity = fixture->GetBody()->GetUserData();
	return
		(subject == entity_id() || fixture_entity != FixtureUserdata(subject)) &&
		(b2ContactFilter::ShouldCollide(&subject_filter, &fixture->GetFilterData()));
}

float32 raycast_input::ReportFixture(
	b2Fixture* const fixture, 
	const b2Vec2& point,
	const b2Vec2& normal, 
	const float32 fraction
) {
	output.intersection = point;

	output.hit = true;
	output.what_entity = fixture->GetBody()->GetUserData();
	output.what_fixture = fixture;
	output.normal = normal;

	if (save_all) {
		outputs.push_back(output);
		return 1.f;
	}

	return fraction;
}

std::vector<physics_raycast_output> physics_world_cache::ray_cast_all_intersections(
	const vec2 p1_meters, 
	const vec2 p2_meters, 
	const b2Filter filter, 
	const entity_id ignore_entity
) const {
	raycast_input callback;
	callback.subject = ignore_entity;
	callback.subject_filter = filter;
	callback.save_all = true;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		//LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
		return callback.outputs;
	}

	b2world->RayCast(&callback, b2Vec2(p1_meters), b2Vec2(p2_meters));
	return callback.outputs;
}

float physics_world_cache::get_closest_wall_intersection(
	const si_scaling si,
	const vec2 position, 
	const float radius, 
	const int ray_amount, 
	const b2Filter filter, 
	const entity_id ignore_entity
) const {
	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		const auto out = ray_cast_px(
			si,
			position, 
			position + vec2::from_degrees((360.f / ray_amount) * i) * radius, 
			filter, 
			ignore_entity
		);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
		}
	}

	return worst_distance;
}

vec2 physics_world_cache::push_away_from_walls(
	const si_scaling si,
	const vec2 position, 
	const float radius, 
	const int ray_amount, 
	const b2Filter filter, 
	const entity_id ignore_entity
) const {
	vec2 resultant;

	float worst_distance = radius;

	for (int i = 0; i < ray_amount; ++i) {
		const auto out = ray_cast_px(
			si,
			position, 
			position + vec2::from_degrees((360.f / ray_amount) * i) * radius, 
			filter, 
			ignore_entity
		);

		if (out.hit) {
			auto diff = (out.intersection - position);
			auto distance = diff.length();

			if (distance < worst_distance) worst_distance = distance;
			resultant += diff;
		}
	}

	if (resultant.is_nonzero()) {
		return position + (-resultant).set_length(radius - worst_distance);
	}
	else {
		return position;
	}
}

physics_raycast_output physics_world_cache::ray_cast(const vec2 p1_meters, const vec2 p2_meters, const b2Filter filter, const entity_id ignore_entity) const {
	raycast_input callback;
	callback.subject = ignore_entity;
	callback.subject_filter = filter;

	if (!((p1_meters - p2_meters).length_sq() > 0.f)) {
		//LOG("Ray casting error: X: %x %x", p1_meters, p2_meters);
		return callback.output;
	}

	b2world->RayCast(&callback, b2Vec2(p1_meters), b2Vec2(p2_meters));
	return callback.output;
}

physics_raycast_output physics_world_cache::ray_cast_px(
	const si_scaling si,
	const vec2 p1, 
	const vec2 p2, 
	const b2Filter filter, 
	const entity_id ignore_entity
) const {
	auto out = ray_cast(si.get_meters(p1), si.get_meters(p2), filter, ignore_entity);
	out.intersection = si.get_pixels(out.intersection);

	return out;
}