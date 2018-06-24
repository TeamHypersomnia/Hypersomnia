#pragma once
#include <algorithm>
#include <sstream>
#include <cmath>

#include "augs/math/rects.h"
#include "augs/math/declare_math.h"
#include "augs/math/arithmetical.h"

#include "augs/templates/hash_templates.h"

#include "augs/string/typesafe_sprintf.h"
#include "augs/string/typesafe_sscanf.h"

template <class T, class = void>
struct has_x_and_y : std::false_type {};

template <class T>
struct has_x_and_y<T, decltype(std::declval<T&>().x, std::declval<T&>().y, void())> : std::true_type {};

template <class T>
constexpr bool has_x_and_y_v = has_x_and_y<T>::value;

namespace augs {
	template <class T>
	basic_ltrb<T> get_aabb_rotated(const basic_vec2<T> initial_size, const T rotation) {
		auto verts = basic_ltrb<T>(0, 0, initial_size.x, initial_size.y).template get_vertices<T>();

		for (auto& v : verts) {
			v.rotate(rotation, initial_size / 2);
		}

		/* expanded aabb that takes rotation into consideration */
		return get_aabb(verts);
	}

	template <class T, class d>
	basic_vec2<T>& rotate_radians(basic_vec2<T>& v, const basic_vec2<T>& origin, const d radians) {
		v -= origin;
		basic_vec2<T> rotated;

		{
			const auto s = static_cast<typename basic_vec2<T>::real>(sin(radians));
			const auto c = static_cast<typename basic_vec2<T>::real>(cos(radians));

			rotated.x = static_cast<T>(v.x * c - v.y * s);
			rotated.y = static_cast<T>(v.x * s + v.y * c);
		}

		v = rotated + origin;

		return v;
	}

	template <class T, class d>
	auto rotate_by_90_multiples(basic_vec2<T>& v, const basic_vec2<T>& origin, const d degrees) {
		if (augs::compare(degrees, static_cast<d>(0))) {
			return static_cast<d>(0);
		}

		if (augs::compare(degrees, static_cast<d>(-90))) {
			v -= origin;
			v = v.perpendicular_ccw();
			v += origin;
			return static_cast<d>(-90);
		}
		if (augs::compare(degrees, static_cast<d>(90))) {
			v -= origin;
			v = v.perpendicular_cw();
			v += origin;
			return static_cast<d>(90);
		}
		if (augs::compare(degrees, static_cast<d>(180))) {
			v -= origin;
			v.neg();
			v += origin;
			return static_cast<d>(180);
		}
		if (augs::compare(degrees, static_cast<d>(-180))) {
			v -= origin;
			v.neg();
			v += origin;
			return static_cast<d>(-180);
		}

		return static_cast<d>(0);
	}

	template <class T, class d>
	auto rotate_degrees_with_90_multiples(basic_vec2<T>& v, const basic_vec2<T>& origin, const d degrees) {
		if (const auto result = rotate_by_90_multiples(v, origin, degrees); result != 0.f) {
			return result;
		}

		const auto radians = DEG_TO_RAD<d> * degrees;
		augs::rotate_radians(v, origin, radians);
		return degrees;
	}

	template <class T, class d>
	auto rotate_radians_with_90_multiples(basic_vec2<T>& v, const basic_vec2<T>& origin, const d radians) {
		const auto degrees = RAD_TO_DEG<d> * radians;

		if (const auto result = rotate_by_90_multiples(v, origin, degrees); result != 0.f) {
			return result * DEG_TO_RAD<d>;
		}

		augs::rotate_radians(v, origin, radians);
		return radians;
	}

	template <class vec, class d>
	vec& rotate(vec& v, const vec& origin, const d angle) {
		return rotate_radians(v, origin, angle * DEG_TO_RAD<d>);
	}
}

template <class type>
struct basic_vec2 {
	// GEN INTROSPECTOR struct basic_vec2 class type
	type x = static_cast<type>(0);
	type y = static_cast<type>(0);
	// END GEN INTROSPECTOR

	static basic_vec2 zero;

	using real = real32;

	void reset() {
		x = static_cast<type>(0);
		y = static_cast<type>(0);
	}

	type bigger_side() const {
		return std::max(x, y);
	}

	basic_vec2 get_sticking_offset(const rectangle_sticking mode) {
		basic_vec2 res;
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
			smaller_p2.distance_from_segment_sq(bigger_p1, bigger_p2) < maximum_offset*maximum_offset
		;
	}

	template <class V>
	basic_vec2(const V& v, std::enable_if_t<has_x_and_y_v<V>>* = nullptr) : 
		x(static_cast<type>(v.x)), 
		y(static_cast<type>(v.y)) 
	{}

	basic_vec2() = default;

	basic_vec2(const type x, const type y) : 
		x(x), 
		y(y) 
	{}

	template <class t>
	basic_vec2& operator=(const basic_vec2<t>& v) {
		x = static_cast<type>(v.x);
		y = static_cast<type>(v.y);
		return *this;
	}

	template <class V, class = std::enable_if_t<has_x_and_y_v<V>>>
	explicit operator V() const {
		return { 
			static_cast<decltype(V::x)>(x), 
			static_cast<decltype(V::y)>(y) 
		};
	}

	static auto square(const type side) {
		return basic_vec2<type>(side, side);
	}

	static auto from_degrees(const real degrees) {
		const auto radians = degrees * DEG_TO_RAD<real>;
		return basic_vec2<type>(static_cast<type>(cos(radians)), static_cast<type>(sin(radians)));
	}

	static auto from_radians(const real radians) {
		return basic_vec2<type>(static_cast<type>(cos(radians)), static_cast<type>(sin(radians)));
	}

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

		auto result = std::acos(dotted);

		return static_cast<real>(result);
	}

	auto full_radians_between(const basic_vec2& v) const {
		return std::atan2(cross(v), dot(v));
	}

	auto full_degrees_between(const basic_vec2& v) const {
		return full_radians_between(v) * RAD_TO_DEG<real>;
	}

	real degrees_between(const basic_vec2& v) const {
		return radians_between(v) * RAD_TO_DEG<real>;
	}

	bool apart_by_less_than_90_degrees(const basic_vec2& b) const {
		return dot(b) >= 0;
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

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	basic_vec2& rotate(const A angle, const basic_vec2 origin) {
		augs::rotate(*this, origin, angle);
		return *this;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	basic_vec2& rotate_radians(const A angle, const basic_vec2 origin) {
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

	basic_vec2& trim_length(const real maximum) {
		const auto actual_length = length();
		normalize_hint(actual_length);

		const auto required_length = std::min(maximum, actual_length);
		return (*this) *= required_length;
	}

	basic_vec2& add_length(const real len) {
		const auto actual_length = length();
		normalize_hint(actual_length);

		const auto required_length = actual_length + len;
		return (*this) *= required_length;
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
	
	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	basic_vec2& discard_fract() {
		x = static_cast<type>(static_cast<int>(x));
		y = static_cast<type>(static_cast<int>(y));
		return *this;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	basic_vec2& round_fract() {
		x = std::round(x);
		y = std::round(y);
		return *this;
	}

	template <class A = type, class = std::enable_if_t<std::is_same_v<float, A>>>
	bool has_fract() const {
		float intpart;

		auto x_fract = modff(x, &intpart);
		auto y_fract = modff(y, &intpart);

		return !(x_fract == 0 && y_fract == 0);
	}

	basic_vec2 perpendicular_ccw() const {
		return basic_vec2(y, -x);
	}

	basic_vec2 perpendicular_cw() const {
		return basic_vec2(-y, x);
	}

	basic_vec2& neg() {
		x = -x;
		y = -y;
		return *this;
	}

	template<class t>
	basic_vec2& shrink(const t len) {
		if (len == static_cast<t>(0)) return *this;

		t current_length = length();

		if (current_length <= len) {
			return *this = basic_vec2(static_cast<type>(0), static_cast<type>(0));
		}

		normalize();
		return (*this) *= (current_length - len);
	}

	basic_vec2& damp(const real delta, const vec2 multipliers) {
		x = augs::damp(x, delta, multipliers.x); 
		y = augs::damp(y, delta, multipliers.y); 

		return *this;
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

	basic_vec2& neg_x() {
		x = -x;
		return *this;
	}

	basic_vec2& neg_y() {
		y = -y;
		return *this;
	}

	basic_vec2& flip() {
		std::swap(x, y);
		return *this;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool x_non_zero(const real eps = AUGS_EPSILON<real>) const {
		return std::abs(x) > eps;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool y_non_zero(const real eps = AUGS_EPSILON<real>) const {
		return std::abs(y) > eps;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool non_zero(const real eps = AUGS_EPSILON<real>) const {
		return x_non_zero(eps) || y_non_zero(eps);
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool neither_zero(const real eps = AUGS_EPSILON<real>) const {
		return x_non_zero(eps) && y_non_zero(eps);
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool is_zero(const real eps = AUGS_EPSILON<real>) const {
		return !non_zero(eps);
	}

	template <class A = type, class = std::enable_if_t<std::is_integral_v<A>>>
	bool non_zero() const {
		return x != 0 || y != 0;
	}

	template <class A = type, class = std::enable_if_t<std::is_integral_v<A>>>
	bool neither_zero() const {
		return x != 0 && y != 0;
	}

	template <class A = type, class = std::enable_if_t<std::is_integral_v<A>>>
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

	bool operator<(const basic_vec2& b) const {
		return length_sq() < b.length_sq();
	}

	bool operator>(const basic_vec2& b) const {
		return length_sq() > b.length_sq();
	}

	template <class v> bool operator==(const v& p) const { return x == p.x && y == p.y; }
	template <class v> bool operator!=(const v& p) const { return x != p.x || y != p.y; }

	basic_vec2 operator-(const basic_vec2& p) const { return basic_vec2(x - p.x, y - p.y); }
	basic_vec2 operator+(const basic_vec2& p) const { return basic_vec2(x + p.x, y + p.y); }
	basic_vec2 operator*(const basic_vec2& p) const { return basic_vec2(x * p.x, y * p.y); }
	basic_vec2 operator/(const basic_vec2& p) const { return basic_vec2(x / p.x, y / p.y); }

	basic_vec2& operator-=(const basic_vec2& p) { x -= p.x; y -= p.y; return *this; }
	basic_vec2& operator+=(const basic_vec2& p) { x += p.x; y += p.y; return *this; }
	basic_vec2& operator*=(const basic_vec2& p) { x *= p.x; y *= p.y; return *this; }
	basic_vec2& operator/=(const basic_vec2& p) { x /= p.x; y /= p.y; return *this; }

	template <class S, class = std::enable_if_t<std::is_arithmetic_v<S>>>
	basic_vec2& operator-=(const S d) { x -= d; y -= d; return *this; }
	template <class S, class = std::enable_if_t<std::is_arithmetic_v<S>>>
	basic_vec2& operator+=(const S d) { x += d; y += d; return *this; }
	template <class S, class = std::enable_if_t<std::is_arithmetic_v<S>>>
	basic_vec2& operator*=(const S d) { x *= d; y *= d; return *this; }
	template <class S, class = std::enable_if_t<std::is_arithmetic_v<S>>>
	basic_vec2& operator/=(const S d) { x /= d; y /= d; return *this; }
};

template <class type, class S, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator-(const basic_vec2<type> t, const S d) { return { t.x - d, t.y - d }; }
template <class type, class S, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator+(const basic_vec2<type> t, const S d) { return { t.x + d, t.y + d }; }
template <class type, class S, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator*(const basic_vec2<type> t, const S d) { return { t.x * d, t.y * d }; }
template <class type, class S, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator/(const basic_vec2<type> t, const S d) { return { t.x / d, t.y / d }; }

template <class S, class type, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator-(const S d, const basic_vec2<type> t) { return { t.x - d, t.y - d }; }
template <class S, class type, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator+(const S d, const basic_vec2<type> t) { return { t.x + d, t.y + d }; }
template <class S, class type, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator*(const S d, const basic_vec2<type> t) { return { t.x * d, t.y * d }; }
template <class S, class type, class = std::enable_if_t<std::is_arithmetic_v<S>>> inline basic_vec2<type> operator/(const S d, const basic_vec2<type> t) { return { t.x / d, t.y / d }; }

template <class type>
basic_vec2<type> basic_vec2<type>::zero = { static_cast<type>(0), static_cast<type>(0) };

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
		return basic_vec2<T>::from_degrees(degrees).degrees();
	}
}

template<class T>
std::ostream& operator<<(std::ostream& out, const basic_vec2<T>& x) {
	return out << typesafe_sprintf("(%x;%x)", x.x, x.y);
}

template<class T>
std::istream& operator>>(std::istream& out, basic_vec2<T>& x) {
	std::string chunk;
	out >> chunk;
	typesafe_sscanf(chunk, "(%x;%x)", x.x, x.y);
	return out;
}