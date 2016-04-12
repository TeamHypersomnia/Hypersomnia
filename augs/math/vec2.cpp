#include "vec2.h"
#include <Box2D\Collision\Shapes\b2CircleShape.h>
#include "ensure.h"

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