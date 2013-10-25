#pragma once
#include <Box2D/Common/b2Math.h>
#include "rects.h"
namespace augmentations {
	template <class type_val, class type_len>
	void damp(type_val& val, type_len len) {
		type_len zero = static_cast<type_len>(0);
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
		angle *= 0.01745329251994329576923690768489;
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

	template <class type = float>
	struct vec2 {
		type x, y;
		
		template <class t>
		static vec2 from_angle(t degrees) {
			vec2 out;
			out.set_from_angle(degrees);
			return out;
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
			return get_radians()*180.0/3.141592653589793238462;
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

		vec2& set_from_angle(float rotation) {
			rotation *= 0.01745329251994329576923690768489;
			set(cos(rotation), sin(rotation));
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
			float len = 1.f/length();
			x *= len;
			y *= len;
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

		template <class v> bool operator < (const v& p) const { return std::make_pair(x, y) < std::make_pair(p.x, p.y); }
		template <class v> bool operator==(const v& p) const { return x == p.x && y == p.y; }
		template <class v> bool operator!=(const v& p) const { return x != p.x || y != p.y; }

		template <class v> vec2 operator-(const v& p) const { return vec2(x - p.x, y - p.y); }
		template <class v> vec2 operator+(const v& p) const { return vec2(x + p.x, y + p.y); }
		template <class v> vec2 operator*(const v& p) const { return vec2(x * p.x, y * p.y); }
		template <class v> vec2 operator/(const v& p) const { return vec2(x / p.x, y / p.y); }

		vec2 operator-(double d) const { return vec2(x - d, y - d); }
		vec2 operator+(double d) const { return vec2(x + d, y + d); }
		vec2 operator*(double d) const { return vec2(x * d, y * d); }
		vec2 operator/(double d) const { return vec2(x / d, y / d); }
		
		vec2 operator-(float d) const { return vec2(x - d, y - d); }
		vec2 operator+(float d) const { return vec2(x + d, y + d); }
		vec2 operator*(float d) const { return vec2(x * d, y * d); }
		vec2 operator/(float d) const { return vec2(x / d, y / d); }
		
		vec2 operator-(int d) const { return vec2(x - d, y - d); }
		vec2 operator+(int d) const { return vec2(x + d, y + d); }
		vec2 operator*(int d) const { return vec2(x * d, y * d); }
		vec2 operator/(int d) const { return vec2(x / d, y / d); }

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

