#pragma once
#include <algorithm>

#include "augs/math/rects.h"
#include "augs/math/declare_math.h"

#include "augs/templates/hash_templates.h"
#include "augs/templates/container_templates.h"

#include "augs/misc/typesafe_sprintf.h"
#include "augs/misc/typesafe_sscanf.h"

template <class T>
constexpr T AUGS_EPSILON = static_cast<T>(0.0001);

template <class T>
constexpr T DEG_TO_RAD = static_cast<T>(0.01745329251994329576923690768489);

template <class T>
constexpr T RAD_TO_DEG = static_cast<T>(1.0 / 0.01745329251994329576923690768489);

template <class T>
constexpr T PI = static_cast<T>(3.1415926535897932384626433832795);

namespace augs {
	template <typename T> 
	int sgn(const T val) {
		return (T(0) < val) - (val < T(0));
	}

	template <class T>
	bool compare(const T a, const T b, const T eps = AUGS_EPSILON<T>) {
		return std::abs(a - b) < eps;
	}

	template <class T, class A>
	T interp(const T a, const T b, const A alpha) {
		return static_cast<T>(a * (static_cast<A>(1) - alpha) + b * (alpha));
	}

	template <class C, class Xp, class Yp>
	auto get_aabb(const C& v, Xp x_pred, Yp y_pred) {
		const auto lower_x = x_pred(minimum_of(v, [&x_pred](const auto a, const auto b) { return x_pred(a) < x_pred(b); }));
		const auto lower_y = y_pred(minimum_of(v, [&y_pred](const auto a, const auto b) { return y_pred(a) < y_pred(b); }));
		const auto upper_x = x_pred(maximum_of(v, [&x_pred](const auto a, const auto b) { return x_pred(a) < x_pred(b); }));
		const auto upper_y = y_pred(maximum_of(v, [&y_pred](const auto a, const auto b) { return y_pred(a) < y_pred(b); }));

		return basic_ltrb<std::remove_const_t<decltype(lower_x)>>(lower_x, lower_y, upper_x, upper_y);
	}

	template <class C>
	auto get_aabb(const C& container) {
		return get_aabb(container,
			[](const auto p) { return p.x; },
			[](const auto p) { return p.y; }
		);
	}

	template <class T>
	basic_ltrb<T> get_aabb_rotated(const basic_vec2<T> initial_size, const T rotation) {
		auto verts = basic_ltrb<T>(0, 0, initial_size.x, initial_size.y).template get_vertices<T>();

		for (auto& v : verts) {
			v.rotate(rotation, initial_size / 2);
		}

		/* expanded aabb that takes rotation into consideration */
		return get_aabb(verts);
	}

	template <class type_val>
	void damp(type_val& val, const type_val len) {
		type_val zero = static_cast<type_val>(0);
		if (val > zero) {
			val -= len;
			
			if (val < zero) {
				val = zero;
			}
		}
		else if (val < zero) {
			val += len;
			
			if (val > zero) {
				val = zero;
			}
		}
	}

	template <typename T>
	T get_clamped(const T a, const T min_a, const T max_a) {
		if (a < min_a) {
			return min_a;
		}

		if (a > max_a) {
			return max_a;
		}

		return a;
	}

	template <typename T>
	void clamp(T& a, const T min_a, const T max_a) {
		a = get_clamped(a, min_a, max_a);
	}

	template <class vec, class d>
	vec& rotate(vec& v, const vec& origin, const d angle) {
		return rotate_radians(v, origin, angle * DEG_TO_RAD<d>);
	}

	template <class T, class d>
	basic_vec2<T>& rotate_radians(basic_vec2<T>& v, const basic_vec2<T>& origin, const d angle) {
		const auto s = static_cast<typename basic_vec2<T>::real>(sin(angle));
		const auto c = static_cast<typename basic_vec2<T>::real>(cos(angle));
		basic_vec2<T> rotated;

		v -= origin;

		rotated.x = static_cast<T>(v.x * c - v.y * s);
		rotated.y = static_cast<T>(v.x * s + v.y * c);

		return v = (rotated + origin);
	}
}

struct b2Vec2;
struct ImVec2;

template <class type>
struct basic_vec2 {
	// GEN INTROSPECTOR struct basic_vec2 class type
	type x;
	type y;
	// END GEN INTROSPECTOR

	template <class B> friend std::ostream& operator<<(std::ostream& out, const basic_vec2<B>& x);
	template <class B> friend std::istream& operator>>(std::istream& out, basic_vec2<B>& x);

	using real = real32;

	void reset() {
		x = static_cast<type>(0);
		y = static_cast<type>(0);
	}

	type bigger_side() const {
		return std::max(x, y);
	}

	basic_vec2 get_sticking_offset(const rectangle_sticking mode) {
		vec2 res;
		switch (mode) {
		case ::rectangle_sticking::LEFT: res = vec2(-x / 2, 0);	break;
		case ::rectangle_sticking::RIGHT: res = vec2(x / 2, 0);	break;
		case ::rectangle_sticking::TOP: res = vec2(0, -y / 2);	break;
		case ::rectangle_sticking::BOTTOM: res = vec2(0, y / 2);	break;
		default: res = vec2(0, 0);			break;
		}

		return res;
	}

	static bool segment_in_segment(
		const basic_vec2 smaller_p1, 
		const basic_vec2 smaller_p2, 
		const basic_vec2 bigger_p1, 
		const basic_vec2 bigger_p2,
		const real maximum_offset
	) {
		return
			((bigger_p2 - bigger_p1).length_sq() >= (smaller_p2 - smaller_p1).length_sq())
			&&
			smaller_p1.distance_from_segment_sq(bigger_p1, bigger_p2) < maximum_offset*maximum_offset
			&&
			smaller_p2.distance_from_segment_sq(bigger_p1, bigger_p2) < maximum_offset*maximum_offset;
	}

	template <class t>
	basic_vec2(
		const basic_vec2<t>& v
	) : 
		x(static_cast<type>(v.x)), 
		y(static_cast<type>(v.y)) 
	{}

	basic_vec2(
		const b2Vec2& v
	) :
		x(static_cast<type>(v.x)),
		y(static_cast<type>(v.y))
	{}

	basic_vec2(
		const ImVec2& v
	) :
		x(static_cast<type>(v.x)),
		y(static_cast<type>(v.y))
	{}

	template <
		class T,
		class = std::enable_if_t<
			std::is_same_v<T, b2Vec2>
		|| std::is_same_v<T, ImVec2>
		>
	>
	operator T() const {
		return { x, y };
	}

	template <class t>
	basic_vec2& operator=(const basic_vec2<t>& v) {
		x = static_cast<type>(v.x);
		y = static_cast<type>(v.y);
		return *this;
	}

	basic_vec2(
		const type x = static_cast<type>(0), 
		const type y = static_cast<type>(0)
	) : 
		x(x), 
		y(y) 
	{}

	/* from http://stackoverflow.com/a/1501725 */
	real distance_from_segment_sq(const basic_vec2 v, const basic_vec2 w) const {
		const auto& p = *this;
		// Return minimum distance between line segment vw and point p
		const real l2 = (v - w).length_sq();  // i.e. |w-v|^2 -  avoid a sqrt
		if (l2 == 0.0) return (p - v).length_sq();   // v == w case
		// Consider the line extending the segment, parameterized as v + t (w - v).
		// We find projection of point p onto the line. 
		// It falls where t = [(p-v) . (w-v)] / |w-v|^2
		const real tt = (p - v).dot(w - v) / l2;
		if (tt < 0.f) return (p - v).length_sq();       // Beyond the 'v' end of the segment
		else if (tt > 1.f) return (p - w).length_sq();  // Beyond the 'w' end of the segment
		const basic_vec2 projection = v + tt * (w - v);  // Projection falls on the segment
		return (p - projection).length_sq();
	}

	real distance_from_segment(const basic_vec2 v, const basic_vec2 w) const {
		return sqrt(distance_from_segment_sq(v, w));
	}

	real get_projection_multiplier(const basic_vec2 start, const basic_vec2 end) const {
		return ((*this) - start).dot(end - start) / (start - end).length_sq();
	}

	basic_vec2 project_onto(const basic_vec2 start, const basic_vec2 end) const {
		return start + get_projection_multiplier(start, end) * (end - start);
	}

	basic_vec2 closest_point_on_segment(const basic_vec2 v, const basic_vec2 w) const {
		const real t = ((*this) - v).dot(w - v) / (v - w).length_sq();

		if (t < 0.f) return v;
		else if (t > 1.f) return w;

		return v + t * (w - v);
	}

	type dot(const basic_vec2 v) const {
		return x * v.x + y * v.y;
	}

	type cross(const basic_vec2 v) const {
		return x * v.y - y * v.x;
	}

	type area() const {
		return x * y;
	}

	type perimeter() const {
		return 2 * x + 2 * y;
	}

	real length() const {
		return static_cast<real>(sqrt(length_sq()));
	}

	type length_sq() const {
		return x*x + y*y;
	}

	real radians() const {
		return static_cast<real>(atan2(y, x));
	}

	real degrees() const {
		return radians()*RAD_TO_DEG<real>;
	}

	real radians_between(const basic_vec2& v) const {
		const auto a_norm = vec2(v).normalize();
		const auto b_norm = vec2(*this).normalize();
		auto dotted = a_norm.dot(b_norm);

		if (dotted > 1) {
			dotted = 1;
		}

		if (dotted < -1) {
			dotted = -1;
		}

		auto result = acos(dotted);

		return static_cast<real>(result);
	}

	real degrees_between(const basic_vec2& v) const {
		return radians_between(v) * RAD_TO_DEG<real>;
	}

	template<class A, class B>
	basic_vec2& set(const A& vx, const B& vy) {
		x = static_cast<type>(vx); y = static_cast<type>(vy);
		return *this;
	}

	template<class v>
	basic_vec2& set(const v& t) {
		set(t.x, t.y);
		return *this;
	}

	basic_vec2& set_from_degrees(const real degrees) {
		const auto radians = degrees * DEG_TO_RAD<real>;
		set(cos(radians), sin(radians));
		normalize();
		return *this;
	}

	basic_vec2& set_from_radians(const real radians) {
		return set_from_degrees(radians * RAD_TO_DEG<real>);
	}

	template <typename v>
	basic_vec2& rotate(const real angle, const v origin) {
		augs::rotate(*this, origin, angle);
		return *this;
	}

	template <typename v>
	basic_vec2& rotate_radians(const real angle, const v origin) {
		augs::rotate_radians(*this, origin, angle);
		return *this;
	}

	basic_vec2 lerp(const basic_vec2& bigger, const real ratio) const {
		return augs::interp(*this, bigger, ratio);
	}

	basic_vec2& set_length(const real len) {
		normalize();
		return (*this) *= len;
	}

	basic_vec2& add_length(const real len) {
		real actual_length = length();
		normalize_hint(actual_length);
		return (*this) *= (actual_length + len);
	}

	basic_vec2& normalize_hint(const real suggested_length) {
		const real len = suggested_length;
		
		if (std::abs(len) < std::numeric_limits<real>::epsilon()) {
			return *this;
		}

		const real inv_len = static_cast<real>(1) / len;

		x = static_cast<type>(static_cast<real>(x) * inv_len);
		y = static_cast<type>(static_cast<real>(y) * inv_len);

		return *this;
	}

	basic_vec2& normalize() {
		return normalize_hint(length());
	}
	
	template <class = std::enable_if_t<!std::is_integral_v<type>>>
	basic_vec2& discard_fract() {
		x = std::trunc(x);
		y = std::trunc(y);
		return *this;
	}

	basic_vec2 perpendicular_cw() const {
		return basic_vec2(-y, x);
	}

	template<class t>
	basic_vec2& damp(const t len) {
		if (len == static_cast<t>(0)) return *this;

		t current_length = length();

		if (current_length <= len) {
			return *this = basic_vec2(static_cast<type>(0), static_cast<type>(0));
		}

		normalize();
		return (*this) *= (current_length - len);
	}

	template<class t>
	basic_vec2& clamp(const basic_vec2<t> rect) {
		if (x > rect.x) x = rect.x;
		if (y > rect.y) y = rect.y;
		if (x < -rect.x) x = -rect.x;
		if (y < -rect.y) y = -rect.y;
		return *this;
	}

	template<class t>
	basic_vec2& clamp_from_zero_to(const basic_vec2<t> rect) {
		if (x > rect.x) x = rect.x;
		if (y > rect.y) y = rect.y;
		if (x < static_cast<type>(0)) x = static_cast<type>(0);
		if (y < static_cast<type>(0)) y = static_cast<type>(0);
		return *this;
	}

	template<class t>
	basic_vec2& clamp_rotated(basic_vec2<t> rect, const t current_angle) {
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

	basic_vec2& clamp(const real max_length) {
		if (length_sq() > max_length*max_length) {
			normalize();
			(*this) *= max_length;
		}
		return *this;
	}

	template <class = std::enable_if_t<std::is_floating_point_v<type>>>
	bool x_non_zero(const real eps = AUGS_EPSILON<real>) const {
		return std::abs(x) > eps;
	}

	template <class = std::enable_if_t<std::is_floating_point_v<type>>>
	bool y_non_zero(const real eps = AUGS_EPSILON<real>) const {
		return std::abs(y) > eps;
	}

	template <class = std::enable_if_t<std::is_floating_point_v<type>>>
	bool non_zero(const real eps = AUGS_EPSILON<real>) const {
		return x_non_zero(eps) || y_non_zero(eps);
	}

	template <class = std::enable_if_t<std::is_floating_point_v<type>>>
	bool is_zero(const real eps = AUGS_EPSILON<real>) const {
		return !non_zero(eps);
	}

	template <class = std::enable_if_t<std::is_integral_v<type>>>
	bool non_zero() const {
		return x != 0 || y != 0;
	}

	template <class = std::enable_if_t<std::is_integral_v<type>>>
	bool is_zero() const {
		return x == 0 && y == 0;
	}

	basic_vec2 operator-() const { return basic_vec2(x * -1, y * -1); }

	bool compare_abs(const basic_vec2& b, const real epsilon = AUGS_EPSILON<real>) const {
		if (std::abs(x - b.x) < epsilon && std::abs(y - b.y) < epsilon) {
			return true;
		}

		return false;
	}

	bool is_epsilon(const real epsilon = AUGS_EPSILON<real>) const {
		if (std::abs(x) < epsilon && std::abs(y) < epsilon) {
			return true;
		}

		return false;
	}

	bool compare(const basic_vec2& b, const real epsilon = AUGS_EPSILON<real>) const {
		if ((*this - b).length_sq() <= epsilon*epsilon) {
			return true;
		}

		return false;
	}

	template <class v> bool operator==(const v& p) const { return x == p.x && y == p.y; }
	template <class v> bool operator!=(const v& p) const { return x != p.x || y != p.y; }

	basic_vec2 operator-(const basic_vec2& p) const { return basic_vec2(x - static_cast<type>(p.x), y - static_cast<type>(p.y)); }
	basic_vec2 operator+(const basic_vec2& p) const { return basic_vec2(x + static_cast<type>(p.x), y + static_cast<type>(p.y)); }
	basic_vec2 operator*(const basic_vec2& p) const { return basic_vec2(x * static_cast<type>(p.x), y * static_cast<type>(p.y)); }
	basic_vec2 operator/(const basic_vec2& p) const { return basic_vec2(x / static_cast<type>(p.x), y / static_cast<type>(p.y)); }

	basic_vec2& operator-=(const basic_vec2& p) { x -= static_cast<type>(p.x); y -= static_cast<type>(p.y); return *this; }
	basic_vec2& operator+=(const basic_vec2& p) { x += static_cast<type>(p.x); y += static_cast<type>(p.y); return *this; }
	basic_vec2& operator*=(const basic_vec2& p) { x *= static_cast<type>(p.x); y *= static_cast<type>(p.y); return *this; }
	basic_vec2& operator/=(const basic_vec2& p) { x /= static_cast<type>(p.x); y /= static_cast<type>(p.y); return *this; }

	basic_vec2& operator-=(const real64 d) { x -= static_cast<type>(d); y -= static_cast<type>(d); return *this; }
	basic_vec2& operator+=(const real64 d) { x += static_cast<type>(d); y += static_cast<type>(d); return *this; }
	basic_vec2& operator*=(const real64 d) { x *= static_cast<type>(d); y *= static_cast<type>(d); return *this; }
	basic_vec2& operator/=(const real64 d) { x /= static_cast<type>(d); y /= static_cast<type>(d); return *this; }

	basic_vec2& operator-=(const real32 d) { x -= static_cast<type>(d); y -= static_cast<type>(d); return *this; }
	basic_vec2& operator+=(const real32 d) { x += static_cast<type>(d); y += static_cast<type>(d); return *this; }
	basic_vec2& operator*=(const real32 d) { x *= static_cast<type>(d); y *= static_cast<type>(d); return *this; }
	basic_vec2& operator/=(const real32 d) { x /= static_cast<type>(d); y /= static_cast<type>(d); return *this; }

	basic_vec2& operator-=(const int d) { x -= static_cast<type>(d); y -= static_cast<type>(d); return *this; }
	basic_vec2& operator+=(const int d) { x += static_cast<type>(d); y += static_cast<type>(d); return *this; }
	basic_vec2& operator*=(const int d) { x *= static_cast<type>(d); y *= static_cast<type>(d); return *this; }
	basic_vec2& operator/=(const int d) { x /= static_cast<type>(d); y /= static_cast<type>(d); return *this; }

	basic_vec2& operator-=(const unsigned d) { x -= static_cast<type>(d); y -= static_cast<type>(d); return *this; }
	basic_vec2& operator+=(const unsigned d) { x += static_cast<type>(d); y += static_cast<type>(d); return *this; }
	basic_vec2& operator*=(const unsigned d) { x *= static_cast<type>(d); y *= static_cast<type>(d); return *this; }
	basic_vec2& operator/=(const unsigned d) { x /= static_cast<type>(d); y /= static_cast<type>(d); return *this; }
};

template <class type> inline basic_vec2<type> operator-(const basic_vec2<type> t, const real64 d) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const basic_vec2<type> t, const real64 d) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const basic_vec2<type> t, const real64 d) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const basic_vec2<type> t, const real64 d) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const basic_vec2<type> t, const real32 d) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const basic_vec2<type> t, const real32 d) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const basic_vec2<type> t, const real32 d) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const basic_vec2<type> t, const real32 d) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const basic_vec2<type> t, const int d) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const basic_vec2<type> t, const int d) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const basic_vec2<type> t, const int d) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const basic_vec2<type> t, const int d) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const basic_vec2<type> t, const unsigned d) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const basic_vec2<type> t, const unsigned d) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const basic_vec2<type> t, const unsigned d) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const basic_vec2<type> t, const unsigned d) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d)}; }

template <class type> inline basic_vec2<type> operator-(const real64 d, const basic_vec2<type> t) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const real64 d, const basic_vec2<type> t) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const real64 d, const basic_vec2<type> t) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const real64 d, const basic_vec2<type> t) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const real32 d, const basic_vec2<type> t) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const real32 d, const basic_vec2<type> t) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const real32 d, const basic_vec2<type> t) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const real32 d, const basic_vec2<type> t) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const int d, const basic_vec2<type> t) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const int d, const basic_vec2<type> t) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const int d, const basic_vec2<type> t) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const int d, const basic_vec2<type> t) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator-(const unsigned d, const basic_vec2<type> t) { return { t.x - static_cast<type>(d), t.y - static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator+(const unsigned d, const basic_vec2<type> t) { return { t.x + static_cast<type>(d), t.y + static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator*(const unsigned d, const basic_vec2<type> t) { return { t.x * static_cast<type>(d), t.y * static_cast<type>(d) }; }
template <class type> inline basic_vec2<type> operator/(const unsigned d, const basic_vec2<type> t) { return { t.x / static_cast<type>(d), t.y / static_cast<type>(d) }; }

namespace std {
	template <class T>
	struct hash<basic_vec2<T>> {
		std::size_t operator()(const basic_vec2<T> v) const {
			return augs::simple_two_hash(v.x, v.y);
		}
	};
}


namespace augs {
	template<class T>
	T normalize_degrees(const T degrees) {
		return basic_vec2<T>().set_from_degrees(degrees).degrees();
	}
}

template<class T>
std::ostream& operator<<(std::ostream& out, const basic_vec2<T>& x) {
	out << typesafe_sprintf("(%x;%x)", x.x, x.y);
	return out;
}

template<class T>
std::istream& operator>>(std::istream& out, basic_vec2<T>& x) {
	std::string chunk;
	out >> chunk;
	typesafe_sscanf(chunk, "(%x;%x)", x.x, x.y);
	return out;
}

struct intersection_output {
	bool hit = false;
	vec2 intersection;
};

intersection_output rectangle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const ltrb rectangle
);

intersection_output circle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const vec2 circle_center,
	const real32 circle_radius
);

intersection_output segment_segment_intersection(
	const vec2 a1,
	const vec2 a2,
	const vec2 b1,
	const vec2 b2
);

std::vector<vec2> generate_circle_points(
	const real32 radius,
	const real32 last_angle_in_degrees,
	const real32 starting_angle_in_degrees,
	const unsigned int number_of_points
);

vec2 position_rectangle_around_a_circle(
	const real32 circle_radius,
	const vec2 rectangle_size,
	const real32 position_at_degrees
);