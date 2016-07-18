#include "vec2.h"
#include <Box2D\Collision\Shapes\b2CircleShape.h>
#include "augs/ensure.h"

std::pair<bool, vec2> circle_ray_intersection(vec2 a, vec2 b, vec2 circle, float radius) {
	b2CircleShape cs;

	cs.m_p = circle;
	cs.m_radius = radius;

	b2RayCastInput in;
	in.maxFraction = 1.f;
	in.p1 = b + (b-a).set_length(4000.f);
	in.p2 = a;

	b2RayCastOutput out;
	b2Transform id;
	id.SetIdentity();

	if (cs.RayCast(&out, in, id, 0))
		return{ true, (in.p1 + vec2(in.p2 - in.p1) * (out.fraction)) };
		
	return{ false, vec2(0, 0) };
}

std::vector<vec2> generate_circle_points(float radius, float last_angle_in_degrees,float starting_angle_in_degrees, unsigned int number_of_points)
{
	std::vector<vec2> result;

	float step = (last_angle_in_degrees - starting_angle_in_degrees) / number_of_points;

	for (float i = starting_angle_in_degrees; i < last_angle_in_degrees; i += step) {
		result.push_back(vec2().set_from_degrees(i).set_length(radius) );
	}

	return result;
}