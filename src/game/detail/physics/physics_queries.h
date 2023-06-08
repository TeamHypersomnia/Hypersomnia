#pragma once
#include <type_traits>

#include <Box2D/Common/b2Math.h>
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2World.h>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include "3rdparty/Box2D/Collision/Shapes/b2CircleShape.h"
#include <Box2D/Collision/b2Collision.h>

#include "game/enums/filters.h"
#include "augs/enums/callback_result.h"

#include "game/detail/physics/physics_queries_declaration.h"

#include "game/detail/physics/shape_helpers.h"

template <class F>
void for_each_in_aabb_meters(
	const b2World& b2world,
	const b2AABB aabb,
	const b2Filter filter,
	F callback
) {
	struct query_aabb_input : b2QueryCallback {
		b2Filter filter;
		F call;

		query_aabb_input(
			F&& f, 
			const b2Filter filter
		) : 
			filter(filter),
			call(std::move(f)) 
		{}

		bool ReportFixture(b2Fixture* fixture) override {
			if (b2ContactFilter::ShouldCollide(&filter, &fixture->GetFilterData())) {
				return call(*fixture) == callback_result::CONTINUE;
			}

			return true;
		}
	};

	auto in = query_aabb_input(std::move(callback), filter);

	b2world.QueryAABB(&in, aabb);
}

template <class S, class F>
void for_each_intersection_with_shape_meters(
	const b2World& b2world,
	const si_scaling si,
	const S& shape,
	const b2Transform queried_shape_transform,
	const b2Filter filter,
	F callback
) {
	b2AABB shape_aabb;
	
	constexpr auto child_index = 0;

	shape.ComputeAABB(
		&shape_aabb, 
		queried_shape_transform,
		child_index
	);

	for_each_in_aabb_meters(
		b2world,
		shape_aabb,
		filter,
		[&](const b2Fixture& fixture) -> callback_result {
			constexpr auto index_a = 0;
			constexpr auto index_b = 0;

			const auto result = b2TestOverlapInfo(
				&shape,
				index_a,
				fixture.GetShape(),
				index_b,
				queried_shape_transform,
				fixture.GetBody()->GetTransform()
			);

			if (result.overlap) {
				return callback(
					fixture,
					si.get_pixels(result.pointA),
					si.get_pixels(result.pointB)
				);
			}
			else {
				return callback_result::CONTINUE;
			}
		}
	);
}

template <class F>
void for_each_intersection_with_shape_meters_generic(
	const b2World& b2world,
	const si_scaling si,
	const b2Shape* const shape,
	const b2Transform queried_shape_transform,
	const b2Filter filter,
	F callback
) {
	for_each_intersection_with_shape_meters(b2world, si, *shape, queried_shape_transform, filter, callback);
}

template <class F>
void for_each_intersection_with_circle_meters(
	const b2World& b2world,
	const si_scaling si,
	const real32 radius_meters,
	const b2Transform queried_shape_transform,
	const b2Filter filter,
	F&& callback
) {
	b2CircleShape shape;
	shape.m_radius = radius_meters;

	for_each_intersection_with_shape_meters(
		b2world,
		si,
		shape,
		queried_shape_transform,
		filter,
		std::forward<F>(callback)
	);
}

template<class F>
void for_each_in_aabb(
	const b2World& b2world,
	const si_scaling si,
	const vec2 p1,
	const vec2 p2,
	const b2Filter filter,
	F callback
) {
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(si.get_meters(p1));
	aabb.upperBound = b2Vec2(si.get_meters(p2));

	for_each_in_aabb_meters(b2world, aabb, filter, callback);
}

template <class F>
void for_each_in_camera(
	const b2World& b2world,
	const si_scaling si,
	const camera_cone cone,
	F callback
) {
	const auto visible_aabb = cone.get_visible_world_rect_aabb();

	for_each_in_aabb(
		b2world,
		si,
		visible_aabb.left_top(),
		visible_aabb.right_bottom(),
		predefined_queries::renderable(),
		callback
	);
}

template <class F>
void for_each_intersection_with_triangle(
	const b2World& b2world,
	const si_scaling si,
	const std::array<vec2, 3> vertices,
	const b2Filter filter,
	F callback
) {
	b2Transform null_transform;
	null_transform.SetIdentity();

	for_each_intersection_with_shape_meters(
		b2world,
		si,
		to_polygon_shape(vertices, si),
		null_transform,
		filter, 
		callback
	);
}

template <class C, class F>
void for_each_intersection_with_polygon(
	const b2World& b2world,
	const si_scaling si,
	const C& vertices,
	const b2Filter filter,
	F callback
) {
	const auto n = static_cast<unsigned>(vertices.size());
	
	const auto average_meters = [&]() {
		auto result = decltype(vertices[0]){};

		for (const auto& v : vertices) {
			result += v;
		}

		result /= n;
		return si.get_meters(result);
	}();

	const auto poly_shape = [&]() {
		b2PolygonShape poly_shape;

		decltype(poly_shape.m_vertices) verts;

		for (std::size_t i = 0; i < n; ++i) {
			const auto meters = si.get_meters(vertices[i]);

			verts[i] = b2Vec2(meters - average_meters);
		}
		
		poly_shape.Set(verts, n);
		return poly_shape;
	}();

	b2Transform queried_transform;
	queried_transform.SetIdentity();
	queried_transform.p = b2Vec2(average_meters);

	for_each_intersection_with_shape_meters(
		b2world,
		si,
		poly_shape,
		queried_transform,
		filter, 
		callback
	);
}

inline auto get_body_entity_that_owns(const b2Fixture& f) {
	return f.GetBody()->GetUserData();
}

inline auto get_entity_that_owns(const b2Fixture& f) {
	return f.GetUserData();
}