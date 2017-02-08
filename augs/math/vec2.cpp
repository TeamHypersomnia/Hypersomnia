#include "vec2.h"
#include <3rdparty/Box2D/Collision/Shapes/b2CircleShape.h>
#include <3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h>
#include "augs/ensure.h"

std::pair<bool, vec2> circle_ray_intersection(
	const vec2 a, 
	const vec2 b, 
	const vec2 circle_center, 
	const float circle_radius
) {
	b2CircleShape cs;

	cs.m_p = circle_center;
	cs.m_radius = circle_radius;

	b2RayCastInput in;
	in.maxFraction = 1.f;
	in.p1 = b + (b-a).set_length(4000.f);
	in.p2 = a;

	b2RayCastOutput out;
	b2Transform id;
	id.SetIdentity();

	if (cs.RayCast(&out, in, id, 0)) {
		return{ true, (in.p1 + vec2(in.p2 - in.p1) * (out.fraction)) };
	}
		
	return{ false, vec2(0, 0) };
}

std::pair<bool, vec2> rectangle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const ltrb rectangle
) {
	b2PolygonShape ps;
	ps.SetAsBox(rectangle.w()/2, rectangle.h()/2);

	const auto center = rectangle.center();

	b2RayCastInput in;
	in.maxFraction = 1.f;
	in.p1 = a - center;
	in.p2 = b - center;

	b2RayCastOutput out;
	b2Transform id;
	id.SetIdentity();

	if (ps.RayCast(&out, in, id, 0)) {
		return{ true, center + (in.p1 + vec2(in.p2 - in.p1) * (out.fraction)) };
	}

	return{ false, vec2(0, 0) };
}

std::vector<vec2> generate_circle_points(
	const float circle_radius, 
	const float last_angle_in_degrees,
	const float starting_angle_in_degrees, 
	const unsigned int number_of_points
) {
	std::vector<vec2> result;

	const float step = (last_angle_in_degrees - starting_angle_in_degrees) / number_of_points;

	for (float i = starting_angle_in_degrees; i < last_angle_in_degrees; i += step) {
		result.push_back(vec2().set_from_degrees(i).set_length(circle_radius) );
	}

	return result;
}
