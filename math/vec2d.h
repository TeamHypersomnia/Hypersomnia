#pragma once
#include <algorithm>
#include <Box2D/Common/b2Math.h>
#include "rects.h"
namespace augmentations {
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

	template <class vec, class d>
	vec& rotate(vec& v, const vec& origin, d angle) {
		angle *= static_cast<d>(0.01745329251994329576923690768489);
		auto s = sin(angle);
		auto c = cos(angle);
		vec rotated;

		v -= origin;

		rotated.x = v.x * c - v.y * s;
		rotated.y = v.x * s + v.y * c;

		return v = (rotated + origin);
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

	template <class type = float> struct vec2;

	template <class type>
	struct vec2 {
		type x, y;
		
		template <class t>
		static vec2 from_degrees(t degrees) {
			vec2 out;
			out.set_from_degrees(degrees);
			return out;
		}

		static bool segment_in_segment(vec2 smaller_p1, vec2 smaller_p2, vec2 bigger_p1, vec2 bigger_p2,
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
		vec2(const t& v) : x(static_cast<type>(v.x)), y(static_cast<type>(v.y)) {}

		template <class t>
		vec2& operator=(const t& v) {
			x = static_cast<type>(v.x);
			y = static_cast<type>(v.y);
			return *this;
		}

		vec2(type x = 0, type y = 0) : x(x), y(y) {}
		vec2(const rects::wh& r) : x(r.w), y(r.h) {
			x = static_cast<type>(r.w);
			y = static_cast<type>(r.h);
		}
		vec2(const rects::ltrb& r) : x(r.l), y(r.t) {}
		vec2(const rects::xywh& r) : x(r.x), y(r.y) {}

		operator b2Vec2() const {
			b2Vec2 t;
			t.x = x;
			t.y = y;
			return t;
		}

		/* from http://stackoverflow.com/a/1501725 */
		type distance_from_segment_sq(vec2 v, vec2 w) const {
			auto& p = *this;
			// Return minimum distance between line segment vw and point p
			const float l2 = (v - w).length_sq();  // i.e. |w-v|^2 -  avoid a sqrt
			if (l2 == 0.0) return (p - v).length_sq();   // v == w case
			// Consider the line extending the segment, parameterized as v + t (w - v).
			// We find projection of point p onto the line. 
			// It falls where t = [(p-v) . (w-v)] / |w-v|^2
			const float t = (p - v).dot(w - v) / l2;
			if (t < 0.0) return (p - v).length_sq();       // Beyond the 'v' end of the segment
			else if (t > 1.0) return (p - w).length_sq();  // Beyond the 'w' end of the segment
			const vec2 projection = v + t * (w - v);  // Projection falls on the segment
			return (p - projection).length_sq();
		}

		type distance_from_segment(vec2 v, vec2 w) const {
			return sqrt(distance_from_segment_sq(v, w));
		}

		vec2 project_onto(vec2 v, vec2 w) const {
			const float t = ((*this) - v).dot(w - v) / (v - w).length_sq();
			return v + t * (w - v);
		}

		float dot(vec2 v) const {
			return x * v.x + y * v.y;
		}

		float length() const {
			return sqrt(length_sq());
		}

		float length_sq() const {
			return x*x + y*y;
		}

		float get_radians() const {
			return atan2(y, x);
		}

		float get_degrees() const {
			return get_radians()*180.0f/3.141592653589793238462f;
		}

		float angle_between(const vec2<>& v) {
			return get_degrees() - v.get_degrees();
		}

		template<class A, class B>
		vec2& set(const A& vx, const B& vy) {
			x = vx; y = vy;
			return *this;
		}
		
		template<class v>
		vec2& set(const v& t) {
			set(t.x, t.y);
			return *this;
		}

		vec2& set_from_degrees(float degrees) {
			float radians = degrees * 0.01745329251994329576923690768489f;
			set(cos(radians), sin(radians));
			normalize();
			return *this;
		}

		template <typename v>
		vec2& rotate(float angle, v origin) {
			augmentations::rotate<vec2, float>(*this, origin, angle);
			return *this;
		}

		vec2 lerp(const vec2& bigger, float ratio) const {
			return (*this) + (bigger - (*this)) * ratio;
		}

		vec2& normalize() {
			float len = length();
			if (len < std::numeric_limits<float>::epsilon()) 
				return *this;
			
			float inv_len = 1.f / len;

			x *= inv_len;
			y *= inv_len;
			return *this;
		}
		
		template<class t>
		vec2& damp(t len) {
			if (len == static_cast<t>(0)) return *this;

			t current_length = length();
			
			if (current_length <= len) {
				return *this = vec2(static_cast<type>(0), static_cast<type>(0));
			}

			normalize();
			return (*this) *= (current_length - len);
		}

		template<class t>
		vec2& clamp(vec2<t> rect) {
			if (x > rect.x) x = rect.x;
			if (y > rect.y) y = rect.y;
			if (x < -rect.x) x = -rect.x;
			if (y < -rect.y) y = -rect.y;
			return *this;
		}

		vec2& clamp(float max_length) {
			if (length_sq() > max_length*max_length) {
				normalize();
				(*this) *= max_length;
			}
			return *this;
		}

		bool non_zero() const {
			return x != type(0) || y != type(0);
		}
		
		vec2 operator-() { return vec2(x * -1, y * -1); }

		bool compare(const vec2& b, const float epsilon = 0.00001f) {
			if (std::abs(x - b.x) < epsilon && std::abs(y - b.y) < epsilon)
				return true;

			return false;
		}

		template <class v> bool operator < (const v& p) const { return std::make_pair(x, y) < std::make_pair(p.x, p.y); }
		template <class v> bool operator==(const v& p) const { return x == p.x && y == p.y; }
		template <class v> bool operator!=(const v& p) const { return x != p.x || y != p.y; }

		template <class v> vec2 operator-(const v& p) const { return vec2(x - p.x, y - p.y); }
		template <class v> vec2 operator+(const v& p) const { return vec2(x + p.x, y + p.y); }
		template <class v> vec2 operator*(const v& p) const { return vec2(x * p.x, y * p.y); }
		template <class v> vec2 operator/(const v& p) const { return vec2(x / p.x, y / p.y); }

		vec2 operator-(double d) const { return vec2(x - static_cast<type>(d), y - static_cast<type>(d)); }
		vec2 operator+(double d) const { return vec2(x + static_cast<type>(d), y + static_cast<type>(d)); }
		vec2 operator*(double d) const { return vec2(x * static_cast<type>(d), y * static_cast<type>(d)); }
		vec2 operator/(double d) const { return vec2(x / static_cast<type>(d), y / static_cast<type>(d)); }
		
		vec2 operator-(float d) const { return vec2(x - d, y - d); }
		vec2 operator+(float d) const { return vec2(x + d, y + d); }
		vec2 operator*(float d) const { return vec2(x * d, y * d); }
		vec2 operator/(float d) const { return vec2(x / d, y / d); }
		
		vec2 operator-(int d) const { return vec2(x - static_cast<type>(d), y - static_cast<type>(d)); }
		vec2 operator+(int d) const { return vec2(x + static_cast<type>(d), y + static_cast<type>(d)); }
		vec2 operator*(int d) const { return vec2(x * static_cast<type>(d), y * static_cast<type>(d)); }
		vec2 operator/(int d) const { return vec2(x / static_cast<type>(d), y / static_cast<type>(d)); }

		template <class v> vec2& operator-=(const v& p) { x -= p.x; y -= p.y; return *this; }
		template <class v> vec2& operator+=(const v& p) { x += p.x; y += p.y; return *this; }
		template <class v> vec2& operator*=(const v& p) { x *= p.x; y *= p.y; return *this; }
		template <class v> vec2& operator/=(const v& p) { x /= p.x; y /= p.y; return *this; }

		vec2& operator-=(double d) { x -= d; y -= d; return *this; }
		vec2& operator+=(double d) { x += d; y += d; return *this; }
		vec2& operator*=(double d) { x *= d; y *= d; return *this; }
		vec2& operator/=(double d) { x /= d; y /= d; return *this; }
		
		vec2& operator-=(float d) { x -= d; y -= d; return *this; }
		vec2& operator+=(float d) { x += d; y += d; return *this; }
		vec2& operator*=(float d) { x *= d; y *= d; return *this; }
		vec2& operator/=(float d) { x /= d; y /= d; return *this; }
		
		vec2& operator-=(int d) { x -= d; y -= d; return *this; }
		vec2& operator+=(int d) { x += d; y += d; return *this; }
		vec2& operator*=(int d) { x *= d; y *= d; return *this; }
		vec2& operator/=(int d) { x /= d; y /= d; return *this; }
	};
}

