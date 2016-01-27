#pragma once
#include <algorithm>
#include "rects.h"
#include "misc/randval.h"
#include "vec2declare.h"

#define AUGS_EPSILON 0.0001f
#define DEG_TO_RAD 0.01745329251994329576923690768489
#define RAD_TO_DEG (1.0/0.01745329251994329576923690768489)

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

namespace augs {
	template <class type_val>
	void damp(type_val& val, type_val len) {
		type_val zero = static_cast<type_val>(0);
		if (val > zero) {
			val -= len;
			if (val < zero)
				val = zero;
		}
		else if (val < zero) {
			val += len;
			if (val > zero)
				val = zero;
		}
	}

	template <typename T>
	T lerp(T a, T b, T alpha) {
		return a + (b - a)*alpha;
	}

	template <typename T>
	void clamp(T& a, T min_a, T max_a) {
		if (a < min_a) a = min_a;
		if (a > max_a) a = max_a;
	}

	template <typename T>
	T get_clamp(T a, T min_a, T max_a) {
		if (a < min_a) return min_a;
		if (a > max_a) return max_a;
		return a;
	}

	template <class vec, class d>
	vec& rotate(vec& v, const vec& origin, d angle) {
		angle *= static_cast<d>(DEG_TO_RAD);
		auto s = sin(angle);
		auto c = cos(angle);
		vec rotated;

		v -= origin;

		rotated.x = v.x * c - v.y * s;
		rotated.y = v.x * s + v.y * c;

		return v = (rotated + origin);
	}

	template <class vec, class d>
	vec from_rotation(vec v, vec origin, d angle) {
		vec new_vec = v;
		rotate(new_vec, origin, angle);
		return new_vec;
	}

	template <class vec>
	vec mult(const vec& a, const vec& b) {
		return vec(a.x * b.x, a.y * b.y);
	}

	template<typename V1, typename V2>
	void set(V1 target, const V2& source) {
		target.x = source.x;
		target.y = source.y;
	}
}

	template <class type>
	struct vec2t {
		type x, y;

		friend std::ostream& operator<< (std::ostream& stream, const vec2t& v) {
			return stream << v.x << "|" << v.y;
		}

		template <class t>
		static vec2t random_on_circle(t radius) {
			return vec2t().set_from_degrees(randval(0.f, 360.f)) * radius;
		}

		static bool segment_in_segment(vec2t smaller_p1, vec2t smaller_p2, vec2t bigger_p1, vec2t bigger_p2,
			type maximum_offset
			) {
				return
					((bigger_p2 - bigger_p1).length_sq() >= (smaller_p2 - smaller_p1).length_sq())
					&&
					smaller_p1.distance_from_segment_sq(bigger_p1, bigger_p2) < maximum_offset*maximum_offset
					&&
					smaller_p2.distance_from_segment_sq(bigger_p1, bigger_p2) < maximum_offset*maximum_offset;
		}

		template <class t>
		vec2t(const t& v) : x(static_cast<type>(v.x)), y(static_cast<type>(v.y)) {}

		template <class t>
		vec2t& operator=(const t& v) {
			x = static_cast<type>(v.x);
			y = static_cast<type>(v.y);
			return *this;
		}

		vec2t(type x = 0, type y = 0) : x(x), y(y) {}

		/* from http://stackoverflow.com/a/1501725 */
		type distance_from_segment_sq(vec2t v, vec2t w) const {
			auto& p = *this;
			// Return minimum distance between line segment vw and point p
			const float l2 = (v - w).length_sq();  // i.e. |w-v|^2 -  avoid a sqrt
			if (l2 == 0.0) return (p - v).length_sq();   // v == w case
			// Consider the line extending the segment, parameterized as v + t (w - v).
			// We find projection of point p onto the line. 
			// It falls where t = [(p-v) . (w-v)] / |w-v|^2
			const float t = (p - v).dot(w - v) / l2;
			if (t < 0.f) return (p - v).length_sq();       // Beyond the 'v' end of the segment
			else if (t > 1.f) return (p - w).length_sq();  // Beyond the 'w' end of the segment
			const vec2t projection = v + t * (w - v);  // Projection falls on the segment
			return (p - projection).length_sq();
		}

		type distance_from_segment(vec2t v, vec2t w) const {
			return sqrt(distance_from_segment_sq(v, w));
		}

		vec2t project_onto(vec2t v, vec2t w) const {
			const float t = ((*this) - v).dot(w - v) / (v - w).length_sq();
			return v + t * (w - v);
		}

		vec2t closest_point_on_segment(vec2t v, vec2t w) const {
			const float t = ((*this) - v).dot(w - v) / (v - w).length_sq();
			
			if (t < 0.f) return v;
			else if (t > 1.f) return w;

			return v + t * (w - v);
		}

		float dot(vec2t v) const {
			return x * v.x + y * v.y;
		}

		float cross(vec2t v) const {
			return x * v.y - y * v.x;
		}

		float length() const {
			return sqrt(length_sq());
		}

		float length_sq() const {
			return x*x + y*y;
		}

		float radians() const {
			return atan2(y, x);
		}

		float degrees() const {
			return radians()*RAD_TO_DEG;
		}

		float angle_between(const vec2t<>& v) {
			return degrees() - v.degrees();
		}

		template<class A, class B>
		vec2t& set(const A& vx, const B& vy) {
			x = vx; y = vy;
			return *this;
		}
		
		template<class v>
		vec2t& set(const v& t) {
			set(t.x, t.y);
			return *this;
		}

		vec2t& set_from_degrees(float degrees) {
			float radians = degrees * DEG_TO_RAD;
			set(cos(radians), sin(radians));
			normalize();
			return *this;
		}

		vec2t& set_from_radians(float radians) {
			return set_from_degrees(radians * RAD_TO_DEG);
		}

		template <typename v>
		vec2t& rotate(float angle, v origin) {
			augs::rotate<vec2t, float>(*this, origin, angle);
			return *this;
		}

		vec2t lerp(const vec2t& bigger, float ratio) const {
			return (*this) + (bigger - (*this)) * ratio;
		}

		vec2t& set_length(float len) {
			normalize();
			return (*this) *= len;
		}

		vec2t& add_length(float len) {
			float actual_length = length();
			normalize_hint(actual_length);
			return (*this) *= (actual_length + len);
		}

		vec2t& normalize_hint(float suggested_length) {
			float len = suggested_length;
			if (std::abs(len) < std::numeric_limits<float>::epsilon())
				return *this;

			float inv_len = 1.f / len;

			x *= inv_len;
			y *= inv_len;
			return *this;
		}

		vec2t& normalize() {
			return normalize_hint(length());
		}

		vec2t perpendicular_cw() {
			return vec2t(-y, x);
		}
		
		template<class t>
		vec2t& damp(t len) {
			if (len == static_cast<t>(0)) return *this;

			t current_length = length();
			
			if (current_length <= len) {
				return *this = vec2t(static_cast<type>(0), static_cast<type>(0));
			}

			normalize();
			return (*this) *= (current_length - len);
		}

		template<class t>
		vec2t& clamp(vec2t<t> rect) {
			if (x > rect.x) x = rect.x;
			if (y > rect.y) y = rect.y;
			if (x < -rect.x) x = -rect.x;
			if (y < -rect.y) y = -rect.y;
			return *this;
		}

		template<class t>
		vec2t& clamp_rotated(vec2t<t> rect, t current_angle) {
			rect.rotate(-current_angle, vec2(0, 0));
			auto unrotated_this = vec2(*this).rotate(-current_angle, vec2(0, 0));

			if (unrotated_this.x > rect.x) unrotated_this.x = rect.x;
			if (unrotated_this.y > rect.y) unrotated_this.y = rect.y;
			if (unrotated_this.x < -rect.x) unrotated_this.x = -rect.x;
			if (unrotated_this.y < -rect.y) unrotated_this.y = -rect.y;
			
			*this = unrotated_this;
			rotate(current_angle, vec2(0, 0));

			return *this;
		}

		vec2t& clamp(float max_length) {
			if (length_sq() > max_length*max_length) {
				normalize();
				(*this) *= max_length;
			}
			return *this;
		}

		bool x_non_zero(float eps = AUGS_EPSILON) const {
			return std::abs(x) > eps;
		}

		bool y_non_zero(float eps = AUGS_EPSILON) const {
			return std::abs(y) > eps;
		}

		bool non_zero(float eps = AUGS_EPSILON) const {
			return x_non_zero(eps) || y_non_zero();
		}
		
		vec2t operator-() { return vec2t(x * -1, y * -1); }

		bool compare_abs(const vec2t& b, const float epsilon = AUGS_EPSILON) {
			if (std::abs(x - b.x) < epsilon && std::abs(y - b.y) < epsilon)
				return true;

			return false;
		}

		bool compare(const vec2t& b, const float epsilon = AUGS_EPSILON) {
			if ((*this - b).length_sq() <= epsilon*epsilon)
				return true;

			return false;
		}

		template <class v> bool operator < (const v& p) const { return std::make_pair(x, y) < std::make_pair(p.x, p.y); }
		template <class v> bool operator==(const v& p) const { return x == p.x && y == p.y; }
		template <class v> bool operator!=(const v& p) const { return x != p.x || y != p.y; }

		template <class v> vec2t operator-(const v& p) const { return vec2t(x - p.x, y - p.y); }
		template <class v> vec2t operator+(const v& p) const { return vec2t(x + p.x, y + p.y); }
		template <class v> vec2t operator*(const v& p) const { return vec2t(x * p.x, y * p.y); }
		template <class v> vec2t operator/(const v& p) const { return vec2t(x / p.x, y / p.y); }

		vec2t operator-(double d) const { return vec2t(x - static_cast<type>(d), y - static_cast<type>(d)); }
		vec2t operator+(double d) const { return vec2t(x + static_cast<type>(d), y + static_cast<type>(d)); }
		vec2t operator*(double d) const { return vec2t(x * static_cast<type>(d), y * static_cast<type>(d)); }
		vec2t operator/(double d) const { return vec2t(x / static_cast<type>(d), y / static_cast<type>(d)); }
		
		vec2t operator-(float d) const { return vec2t(x - d, y - d); }
		vec2t operator+(float d) const { return vec2t(x + d, y + d); }
		vec2t operator*(float d) const { return vec2t(x * d, y * d); }
		vec2t operator/(float d) const { return vec2t(x / d, y / d); }
		
		vec2t operator-(int d) const { return vec2t(x - static_cast<type>(d), y - static_cast<type>(d)); }
		vec2t operator+(int d) const { return vec2t(x + static_cast<type>(d), y + static_cast<type>(d)); }
		vec2t operator*(int d) const { return vec2t(x * static_cast<type>(d), y * static_cast<type>(d)); }
		vec2t operator/(int d) const { return vec2t(x / static_cast<type>(d), y / static_cast<type>(d)); }

		template <class v> vec2t& operator-=(const v& p) { x -= p.x; y -= p.y; return *this; }
		template <class v> vec2t& operator+=(const v& p) { x += p.x; y += p.y; return *this; }
		template <class v> vec2t& operator*=(const v& p) { x *= p.x; y *= p.y; return *this; }
		template <class v> vec2t& operator/=(const v& p) { x /= p.x; y /= p.y; return *this; }

		vec2t& operator-=(double d) { x -= d; y -= d; return *this; }
		vec2t& operator+=(double d) { x += d; y += d; return *this; }
		vec2t& operator*=(double d) { x *= d; y *= d; return *this; }
		vec2t& operator/=(double d) { x /= d; y /= d; return *this; }
		
		vec2t& operator-=(float d) { x -= d; y -= d; return *this; }
		vec2t& operator+=(float d) { x += d; y += d; return *this; }
		vec2t& operator*=(float d) { x *= d; y *= d; return *this; }
		vec2t& operator/=(float d) { x /= d; y /= d; return *this; }
		
		vec2t& operator-=(int d) { x -= d; y -= d; return *this; }
		vec2t& operator+=(int d) { x += d; y += d; return *this; }
		vec2t& operator*=(int d) { x *= d; y *= d; return *this; }
		vec2t& operator/=(int d) { x /= d; y /= d; return *this; }
	};

