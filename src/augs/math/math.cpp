#include "augs/math/math.h"
#include <3rdparty/Box2D/Collision/Shapes/b2CircleShape.h>
#include <3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h>
#include <3rdparty/Box2D/Collision/Shapes/b2EdgeShape.h>
#include "augs/ensure.h"

bool point_in_rect(
	const vec2 center,
	const real32 rotation,
	const vec2 size,
	vec2 point
) {
	point.rotate(-rotation, center);

	return ltrb::center_and_size(center, size).hover(point);
}

intersection_output circle_ray_intersection(
	const vec2 a, 
	const vec2 b, 
	const vec2 circle_center, 
	const real32 circle_radius
) {
	b2CircleShape cs;

	cs.m_p = b2Vec2(circle_center);
	cs.m_radius = circle_radius;

	b2RayCastInput in;
	in.maxFraction = 1.f;
	in.p1 = b2Vec2(a);
	in.p2 = b2Vec2(b);

	b2RayCastOutput out;
	b2Transform id;
	id.SetIdentity();

	if (cs.RayCast(&out, in, id, 0)) {
		intersection_output result;
		
		result.hit = true;
		result.intersection = vec2(in.p1) + vec2(in.p2 - in.p1) * (out.fraction);
		
		return result;
	}
		
	return intersection_output();
}

intersection_output rectangle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const ltrb rectangle
) {
	b2PolygonShape ps;
	ps.SetAsBox(rectangle.w()/2, rectangle.h()/2);

	const auto center = rectangle.get_center();

	b2RayCastInput in;
	in.maxFraction = 1.f;
	in.p1 = b2Vec2(a - center);
	in.p2 = b2Vec2(b - center);

	b2RayCastOutput out;
	b2Transform id;
	id.SetIdentity();

	if (ps.RayCast(&out, in, id, 0)) {
		intersection_output result;
		
		result.hit = true;
		result.intersection = center + vec2(vec2(in.p1) + vec2(in.p2 - in.p1) * (out.fraction));
		
		return result;
	}

	return intersection_output();
}

std::vector<vec2> generate_circle_points(
	const real32 circle_radius, 
	const real32 last_angle_in_degrees,
	const real32 starting_angle_in_degrees, 
	const unsigned int number_of_points
) {
	std::vector<vec2> result;

	const real32 step = (last_angle_in_degrees - starting_angle_in_degrees) / number_of_points;

	for (real32 i = starting_angle_in_degrees; i < last_angle_in_degrees; i += step) {
		result.push_back(vec2::from_degrees(i).set_length(circle_radius) );
	}

	return result;
}

intersection_output segment_segment_intersection(
	const vec2 a1,
	const vec2 a2,
	const vec2 b1,
	const vec2 b2
) {
	/* prepare b2RayCastOutput/b2RayCastInput data for raw b2EdgeShape::RayCast call */
	b2RayCastOutput output;
	b2RayCastInput input;
	output.fraction = 0.f;
	input.maxFraction = 1.0;
	input.p1 = b2Vec2(a1);
	input.p2 = b2Vec2(a2);

	/* we don't need to transform edge or ray since they are in the same space
	but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
	*/
	b2Transform null_transform;
	null_transform.SetIdentity();

	b2EdgeShape b2edge;
	b2edge.Set(b2Vec2(b1), b2Vec2(b2));

	intersection_output out;
	out.hit = b2edge.RayCast(&output, input, null_transform, 0);
	out.intersection = input.p1 + output.fraction * (input.p2 - input.p1);

	return out;
}

vec2 position_rectangle_around_a_circle(
	const real32 circle_radius,
	const vec2 rectangle_size,
	const real32 position_at_degrees
) {
	const vec2 top_bounds[2] = { vec2(-rectangle_size.x / 2, -circle_radius - rectangle_size.y / 2), vec2(rectangle_size.x / 2, -circle_radius - rectangle_size.y / 2) };
	const vec2 left_bounds[2] = { vec2(-circle_radius - rectangle_size.x / 2, rectangle_size.y / 2), vec2(-circle_radius - rectangle_size.x / 2, -rectangle_size.y / 2) };
	const vec2 bottom_bounds[2] = { top_bounds[1] * vec2(1, -1), top_bounds[0] * vec2(1, -1) };
	const vec2 right_bounds[2] = { left_bounds[1] * vec2(-1, 1), left_bounds[0] * vec2(-1, 1) };

	const vec2 all_bounds[4][2] = {
		{ left_bounds[0], left_bounds[1] },
		{ top_bounds[0], top_bounds[1] },
		{ right_bounds[0], right_bounds[1] },
		{ bottom_bounds[0], bottom_bounds[1] }
	};

	const vec2 angle_norm = vec2::from_degrees(position_at_degrees);

	static const vec2 quadrant_multipliers[4] = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1), };
	static const vec2 quadrants_on_circle[4] = { vec2(-1, 0), vec2(0, -1), vec2(1, 0), vec2(0, 1), };

	for (int i = 0; i < 4; ++i) {
		const auto a = vec2(all_bounds[i][0]).normalize();
		const auto b = vec2(all_bounds[i][1]).normalize();
		const auto c = vec2(all_bounds[(i + 1) % 4][0]).normalize();
		const auto v = angle_norm;

		real32 bound_angular_distance = a.cross(b);
		real32 target_angular_distance = a.cross(v);

		if (target_angular_distance >= 0 && b.cross(v) <= 0) {
			return augs::interp(all_bounds[i][0], all_bounds[i][1], target_angular_distance / bound_angular_distance);
		}
		else {
			bound_angular_distance = b.cross(c);
			target_angular_distance = b.cross(v);

			if (target_angular_distance >= 0.0 && c.cross(v) <= 0.0) {
				return vec2(quadrants_on_circle[i]).rotate(
					(target_angular_distance / bound_angular_distance) * 90
				) * circle_radius + quadrant_multipliers[i] * rectangle_size / 2;
			}
		}
	}

	//ensure(false);
	return vec2(0, 0);
}


vec2 clamp_with_raycast(
	const vec2 point,
	const vec2 rectangle_size
) {
	const auto raycasted_aabb = ltrb::center_and_size(vec2::zero, rectangle_size);

	const auto raycast_a = vec2::zero;
	const auto raycast_b = point;

	const auto edges = raycasted_aabb.make_edges();

	for (std::size_t e = 0; e < edges.size(); ++e) {
		const auto intersection = ::segment_segment_intersection(
			raycast_a,
			raycast_b,
			edges[e][0],
			edges[e][1]
		);

		if (intersection.hit) {
			return intersection.intersection;
		}
	}

	return point;
}