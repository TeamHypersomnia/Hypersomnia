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

	inline auto calc_homing_vel(
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

	inline auto calc_homing_correction_vel(
		const vec2 current_vel,
		const vec2 target_vector
	) {
		const auto best_perpendicular = calc_homing_vel(current_vel, target_vector);

		if (current_vel.apart_by_less_than_90_degrees(target_vector)) {
			if (const auto intersect = line_line_intersect(
				current_vel, current_vel + current_vel.perpendicular_cw(),
				vec2::zero, target_vector
			)) {
				const auto to_intersection_vel = *intersect - current_vel;
				//LOG_NVPS(current_vel, best_perpendicular, target_vector, *intersect, to_intersection_vel);
				return std::min(best_perpendicular, to_intersection_vel);
			}
		}

		return best_perpendicular;
	}

	inline auto calc_homing(
		const vec2 current_vel,
		const vec2 current_pos,
		const vec2 target_pos
	) {
		const auto target_vector = target_pos - current_pos;
		return calc_homing_vel(current_vel, target_vector).set_length(target_vector.length());
	}

	inline auto calc_danger_avoidance(
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

	inline auto calc_danger_avoidance_proportional(
		const vec2 victim_pos,
		const vec2 danger_pos,
		const vec2 danger_vel,
		const real32 comfort_zone,
		const real32 avoidance_force
	) {
		const auto avoidance = calc_danger_avoidance(victim_pos, danger_pos, danger_vel);
		const auto danger_dist = (victim_pos - danger_pos).length();

		return vec2(avoidance).set_length(avoidance_force * std::max(0.f, 1.f - (danger_dist / comfort_zone)));
	}
}
