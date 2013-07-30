#pragma once
#include "rects.h"
namespace augmentations {
	template <class vec, class d>
	vec& rotate(vec& v, const vec& origin, d degrees) {
		float s = sin(degrees);
		float c = cos(degrees);
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
		vec2(const t& v) : x(v.x), y(v.y) {}

		template <class t>
		vec2& operator=(const t& v) {
			x = v.x;
			y = v.y;
			return *this;
		}

		vec2(type x = 0, type y = 0) : x(x), y(y) {}
		vec2(const rects::wh& r) : x(r.w), y(r.h) {
			x = r.w;
			y = r.h;
		}
		vec2(const rects::ltrb& r) : x(r.l), y(r.t) {}
		vec2(const rects::xywh& r) : x(r.x), y(r.y) {}

		template <typename v>
		operator v() const {
			v t;
			t.x = x;
			t.y = y;
			return t;
		}

		float length() const {
			return sqrt(x*x + y*y);
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
			set(sin(rotation), cos(rotation));
			normalize();
			return *this;
		}

		template <typename v>
		vec2& rotate(float degrees, v origin) {
			augmentations::rotate(*this, origin, degrees);
			return *this;
		}

		vec2& normalize() {
			float len = 1.f/length();
			x *= len;
			y *= len;
			return *this;
		}

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

