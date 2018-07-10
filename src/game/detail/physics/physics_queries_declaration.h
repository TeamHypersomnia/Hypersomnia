#pragma once
#include <array>

#include "augs/math/si_scaling.h"
#include "augs/math/camera_cone.h"

struct b2AABB;
struct b2Filter;
class b2Shape;
class b2World;
struct b2Transform;

template <class F>
void for_each_in_aabb_meters(
	const b2World& b2world,
	const b2AABB aabb,
	const b2Filter filter,
	F callback
);

template <class F>
void for_each_intersection_with_shape_meters(
	const b2World& b2world,
	const si_scaling si,
	const b2Shape* const shape,
	const b2Transform queried_shape_transform,
	const b2Filter filter,
	F callback
);

template<class F>
void for_each_in_aabb(
	const b2World& b2world,
	const si_scaling si,
	const vec2 p1,
	const vec2 p2,
	const b2Filter filter,
	F callback
);

template <class F>
void for_each_in_camera(
	const b2World& b2world,
	const si_scaling si,
	const camera_eye camera,
	const vec2 screen_size,
	F callback
);

template <class F>
void for_each_intersection_with_triangle(
	const b2World& b2world,
	const si_scaling si,
	const std::array<vec2, 3> vertices,
	const b2Filter filter,
	F callback
);

template <class C, class F>
void for_each_intersection_with_polygon(
	const b2World& b2world,
	const si_scaling si,
	const C& vertices,
	const b2Filter filter,
	F callback
);