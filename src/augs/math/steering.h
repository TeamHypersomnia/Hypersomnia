#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/log.h"

namespace augs {
	///Calculate intersection of two lines.
	///\return true if found, false if not found or error
	inline bool LineLineIntersect(double x1, double y1, //Line 1 start
	double x2, double y2, //Line 1 end
	double x3, double y3, //Line 2 start
	double x4, double y4, //Line 2 end
	double &ixOut, double &iyOut) //Output 
	{
		auto Det = [](double a, double b, double c, double d)
		{
			return a*d - b*c;
		};
		//http://mathworld.wolfram.com/Line-LineIntersection.html

		double detL1 = Det(x1, y1, x2, y2);
		double detL2 = Det(x3, y3, x4, y4);
		double x1mx2 = x1 - x2;
		double x3mx4 = x3 - x4;
		double y1my2 = y1 - y2;
		double y3my4 = y3 - y4;

		double xnom = Det(detL1, x1mx2, detL2, x3mx4);
		double ynom = Det(detL1, y1my2, detL2, y3my4);
		double denom = Det(x1mx2, y1my2, x3mx4, y3my4);
		if(denom == 0.0)//Lines don't seem to cross
		{
			ixOut = NAN;
			iyOut = NAN;
			return false;
		}

		ixOut = xnom / denom;	
		iyOut = ynom / denom;
		if(!isfinite(ixOut) || !isfinite(iyOut)) //Probably a numerical issue
		return false;

		return true; //All OK
	}

	inline std::optional<vec2> line_line_intersect(
		const vec2 p1, const vec2 p2,
		const vec2 p3, const vec2 p4
	) {
		double xout, yout;

		if (LineLineIntersect(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y,
			xout, yout
		)) {
			return vec2(xout, yout);
		}

		return std::nullopt;
	}

	inline bool isLeft(const vec2 a, const vec2 b, const vec2 c) {
		return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
	}

	inline auto seek(
		const vec2 current_vel,
		const vec2 current_pos,
		const vec2 target_pos
	) {
		const auto desired_vel = target_pos - current_pos;
		return desired_vel - current_vel;
	}

	inline auto seek(
		const vec2 current_vel,
		const vec2 current_pos,
		const vec2 hostile_pos,
		const real32 desired_speed
	) {
		const auto desired_vel = (hostile_pos - current_pos).set_length(desired_speed);
		return desired_vel - current_vel;
	}

	inline auto arrive(
		const vec2 current_vel,
		const vec2 current_pos,
		const vec2 arrival_pos,
		const real32 max_desired_speed,
		const real32 easing_zone
	) {
		const auto target_vector = arrival_pos - current_pos;
		const auto dist_from_target = target_vector.length();

		const auto desired_speed = std::min(1.f, dist_from_target / easing_zone) * max_desired_speed;
		const auto target_dir = target_vector / dist_from_target;
		const auto desired_vel = target_dir * desired_speed;

		return desired_vel - current_vel;
	}

	inline auto furthest_perpendicular(
		const vec2 current_vel,
		const vec2 target_vector
	) {
		const auto right_hand = current_vel.perpendicular_cw();
		const auto left_hand = -current_vel.perpendicular_cw();

		if (right_hand.dot(target_vector) < left_hand.dot(target_vector)) {
			return right_hand;
		}

		return left_hand;
	}

	inline auto closest_perpendicular(
		const vec2 current_vel,
		const vec2 target_vector
	) {
		const auto right_hand = current_vel.perpendicular_cw();
		const auto left_hand = -current_vel.perpendicular_cw();

		if (right_hand.dot(target_vector) > left_hand.dot(target_vector)) {
			return right_hand;
		}

		return left_hand;
	}

	inline auto immediate_avoidance(
		const vec2 victim_pos,
		const vec2 victim_vel,
		const vec2 danger_pos,
		const vec2,
		const real32 comfort_zone,
		const real32 desired_speed
	) {
		const auto danger_dist = (danger_pos - victim_pos).length();
		const auto comfort_disturbance = std::max(0.f, 1.f - danger_dist / comfort_zone);

		const auto target_vector = danger_pos - victim_pos;
		const auto desired_vel = furthest_perpendicular(victim_vel, target_vector).set_length(desired_speed);

		return (desired_vel - victim_vel) * comfort_disturbance;
	}

	inline auto ease_against_projection(
		const vec2 current_vel,
		const vec2 best_perpendicular_vel,
		const vec2 target_vector
	) {
		if (current_vel.apart_by_less_than_90_degrees(target_vector)) {
			if (const auto intersect = line_line_intersect(
				current_vel, current_vel + current_vel.perpendicular_cw(),
				vec2::zero, target_vector
			)) {
				const auto to_intersection_vel = *intersect - current_vel;
				//LOG_NVPS(current_vel, best_perpendicular_vel, target_vector, *intersect, to_intersection_vel);
				return std::min(best_perpendicular_vel, to_intersection_vel);
			}
		}

		return best_perpendicular_vel;
	}

	inline auto homing_correction(
		const vec2 current_vel,
		const vec2 target_vector
	) {
		const auto best_perpendicular_vel = closest_perpendicular(current_vel, target_vector);
		return ease_against_projection(current_vel, best_perpendicular_vel, target_vector);
	}

	inline auto homing_correction_in_radius(
		const vec2 current_vel,
		const vec2 current_pos,
		const vec2 target_pos,
		const real32 detection_radius
	) {
		const auto target_vector = target_pos - current_pos;

		return homing_correction(
			current_vel * std::min(1.f, target_vector.length() / detection_radius),
			target_vector
		);
	}

	inline auto danger_avoidance(
		const vec2 victim_pos,
		const vec2 danger_pos,
		const vec2 danger_vel
	) {
		const auto danger_speed = danger_vel.length();
		const auto danger_dir = (danger_pos - victim_pos);

		if (danger_speed > 10) {
			return isLeft(danger_pos, danger_pos + danger_vel, victim_pos) ? danger_vel.perpendicular_cw() : -danger_vel.perpendicular_cw();
		}

		return -danger_dir;
	}

	inline auto danger_avoidance_in_comfort_zone(
		const vec2 victim_pos,
		const vec2 danger_pos,
		const vec2 danger_vel,
		const real32 comfort_zone,
		const real32 avoidance_force
	) {
		const auto avoidance = danger_avoidance(victim_pos, danger_pos, danger_vel);
		const auto danger_dist = (victim_pos - danger_pos).length();

		return vec2(avoidance).set_length(avoidance_force * std::max(0.f, 1.f - (danger_dist / comfort_zone)));
	}

	inline auto steer_to_avoid_bounds(
		const vec2 velocity,
		const vec2 position,
		const xywh& bound,
		const real32 max_dist_to_avoid,
		const real32 force_multiplier
	) {
		const auto edges = bound.make_edges();
		auto dist_to_closest = std::numeric_limits<real32>::max();

		for (const auto& e : edges) {
			const auto dist = position.distance_from_segment_sq(e[0], e[1]);

			if (dist < dist_to_closest) {
				dist_to_closest = dist;
			}
		}

		dist_to_closest = std::sqrt(dist_to_closest);

		/* Finally, correct velocities against the walls */

		auto dir_mult = std::max(0.f, 1.f - dist_to_closest / max_dist_to_avoid) * force_multiplier;

		if (!bound.hover(position)) {
			/* Protect from going outside */
			dir_mult = 1.f;
		}

		return augs::seek(
			velocity,
			position,
			bound.get_center(),
			velocity.length()
		) * dir_mult;
	}

}
