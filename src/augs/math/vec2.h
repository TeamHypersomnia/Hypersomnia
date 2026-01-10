#pragma once
#include <cstddef>
#include "augs/math/repro_math.h"

#include <string>
#include <algorithm>
#include <cmath>

#include "augs/math/rects.h"
#include "augs/math/declare_math.h"
#include "augs/math/arithmetical.h"

#include "augs/templates/hash_templates.h"

template <class T, class = void>
struct has_x_and_y : std::false_type {};

template <class T>
struct has_x_and_y<T, decltype(std::declval<T&>().x, std::declval<T&>().y, void())> : std::true_type {};

template <class T>
constexpr bool has_x_and_y_v = has_x_and_y<T>::value;

template <class T>
const std::string& get_type_name();

template <class type>
struct basic_transform;

template <class type>
struct basic_vec2 {
	using coordinate_type = type;
	using segment_type = std::array<basic_vec2, 2>;

	static std::string get_custom_type_name() {
		return "vec2_" + get_type_name<type>();
	}

	// GEN INTROSPECTOR struct basic_vec2 class type
	type x = static_cast<type>(0);
	type y = static_cast<type>(0);
	// END GEN INTROSPECTOR

	static const basic_vec2 zero;

	using real = real32;

	void reset() {
		x = static_cast<type>(0);
		y = static_cast<type>(0);
	}

	type bigger_side() const {
		return std::max(x, y);
	}

	type smaller_side() const {
		return std::min(x, y);
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

	template <class V>
	basic_vec2(const V& v, std::enable_if_t<has_x_and_y_v<V>>* = nullptr) : 
		x(static_cast<type>(v.x)), 
		y(static_cast<type>(v.y)) 
	{}

	basic_vec2() = default;

	constexpr basic_vec2(const type x, const type y) : 
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

	template <class V>
	static auto scaled_to_max_size(const V& original_size, const type max_size) {
		return basic_vec2(basic_vec2<real>(original_size) * (static_cast<real>(max_size) / original_size.bigger_side()));
	}

	static auto from_degrees(const real degrees) {
		const auto radians = degrees * DEG_TO_RAD<real>;

		real32 sx = 0.f;
		real32 cx = 0.f;

		repro::sincosf(radians, sx, cx);

		return basic_vec2<type>(cx, sx);
	}

	static auto from_radians(const real radians) {
		real32 sx = 0.f;
		real32 cx = 0.f;

		repro::sincosf(radians, sx, cx);

		return basic_vec2<type>(cx, sx);
	}

	real distance_from(const segment_type s) const {
		return repro::sqrt(sq_distance_from(s));
	}

	real get_projection_multiplier(const basic_vec2 start, const basic_vec2 end) const {
		const auto segment = end - start;
		const auto proj_mult = ((*this) - start).dot(segment) / segment.length_sq();

		return proj_mult;
	}

	auto project_onto(const basic_vec2 start, const basic_vec2 end) const {
		const auto segment = end - start;
		const auto proj_mult = ((*this) - start).dot(segment) / segment.length_sq();

		return start + proj_mult * segment;
	}

	auto closest_point_on_line(const basic_vec2 start, const basic_vec2 end) const {
		const auto segment = end - start;
		const auto proj_mult = ((*this) - start).dot(segment) / segment.length_sq();

		return start + proj_mult * segment;
	}

	auto closest_point_on_line(const segment_type s) const {
		return closest_point_on_line(s[0], s[1]);
	}

	auto closest_point_on_segment(const basic_vec2 start, const basic_vec2 end) const {
		const auto segment = end - start;
		const auto proj_mult = ((*this) - start).dot(segment) / segment.length_sq();

		if (proj_mult <= 0.f) {
			return start;
		}

		if (proj_mult >= 1.f) {
			return end;
		}

		return start + proj_mult * segment;
	}

	real sq_distance_from_segment(const basic_vec2 start, const basic_vec2 end) const {
		const auto& p = *this;

		if (start.compare(end)) {
			return (p - start).length_sq();
		}

		return (p - closest_point_on_segment(start, end)).length_sq();
	}

	auto sq_distance_from(const segment_type segment) const {
		return sq_distance_from_segment(segment[0], segment[1]);
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
		return static_cast<real>(repro::sqrt(length_sq()));
	}

	type length_sq() const {
		return x*x + y*y;
	}

	real radians() const {
		if constexpr(std::is_integral_v<type>) {
			return static_cast<real>(repro::atan2(real(y), real(x)));
		}
		else {
			return static_cast<real>(repro::atan2(y, x));
		}
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

		auto result = repro::acos(dotted);

		return static_cast<real>(result);
	}

	auto full_radians_between(const basic_vec2& v) const {
		return repro::atan2(cross(v), dot(v));
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

	basic_vec2& normalize_hint(const real len) {
		if (repro::fabs(len) < std::numeric_limits<real>::epsilon()) {
			return *this;
		}

		const auto inv_len = static_cast<real>(1) / len;

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
		x = repro::round(x);
		y = repro::round(y);
		return *this;
	}

	template <class A = type, class = std::enable_if_t<std::is_same_v<float, A>>>
	bool has_fract() const {
		float intpart;

		auto x_fract = modff(x, &intpart);
		auto y_fract = modff(y, &intpart);

		return !(x_fract == 0 && y_fract == 0);
	}

	basic_vec2 transposed() const {
		return basic_vec2(y, x);
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
		rect.rotate(-current_angle);
		auto unrotated_this = vec2(*this).rotate(-current_angle);

		if (unrotated_this.x > rect.x) unrotated_this.x = rect.x;
		if (unrotated_this.y > rect.y) unrotated_this.y = rect.y;
		if (unrotated_this.x < -rect.x) unrotated_this.x = -rect.x;
		if (unrotated_this.y < -rect.y) unrotated_this.y = -rect.y;

		*this = unrotated_this;
		rotate(current_angle);

		return *this;
	}

	basic_vec2& clamp(const real max_length) {
		if (length_sq() > max_length*max_length) {
			normalize();
			(*this) *= max_length;
		}
		return *this;
	}

	basic_vec2& lessen(const basic_vec2& b) {
		x = std::min(x, b.x);
		y = std::min(y, b.y);
		return *this;
	}

	basic_vec2& biggen(const basic_vec2& b) {
		x = std::max(x, b.x);
		y = std::max(y, b.y);
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

	basic_vec2& reflect(const basic_vec2& around_normal) {
		const auto& d = *this;
		const auto& n = around_normal;

		*this = (d - 2 * d.dot(n) * n);
		return *this;
	}

	basic_vec2& flip() {
		std::swap(x, y);
		return *this;
	}

	template <class R>
	auto& rotate_radians(const R radians) {
		static_assert(std::is_floating_point_v<R>);

		real32 s = 0.f;
		real32 c = 0.f;

		repro::sincosf(radians, s, c);

		const auto new_x = x * c - y * s;
		const auto new_y = x * s + y * c;

		x = static_cast<type>(new_x);
		y = static_cast<type>(new_y);

		return *this;
	}

	template <class R>
	auto& rotate_radians(const R radians, const basic_vec2& origin) {
		static_assert(std::is_floating_point_v<R>);

		*this -= origin;
		rotate_radians(radians);
		*this += origin;
		return *this;
	}

	template <class D>
	auto rotate_by_90_multiples(const D in_degrees, const basic_vec2& origin) {
		auto& v = *this;

		const auto degrees = augs::normalize_degrees(in_degrees);

		if (augs::compare(degrees, static_cast<D>(0))) {
			return static_cast<D>(0);
		}

		if (augs::compare(degrees, static_cast<D>(-90))) {
			v -= origin;
			v = perpendicular_ccw();
			v += origin;
			return static_cast<D>(-90);
		}
		if (augs::compare(degrees, static_cast<D>(90))) {
			v -= origin;
			v = perpendicular_cw();
			v += origin;
			return static_cast<D>(90);
		}
		if (augs::compare(degrees, static_cast<D>(180))) {
			v -= origin;
			neg();
			v += origin;
			return static_cast<D>(180);
		}
		if (augs::compare(degrees, static_cast<D>(-180))) {
			v -= origin;
			neg();
			v += origin;
			return static_cast<D>(-180);
		}

		return static_cast<D>(0);
	}

	template <class D>
	auto rotate_degrees_with_90_multiples(const D degrees, const basic_vec2& origin) {
		if (const auto result = rotate_by_90_multiples(degrees, origin); result != 0.f) {
			return result;
		}

		const auto radians = DEG_TO_RAD<D> * degrees;
		rotate_radians(radians, origin);
		return degrees;
	}

	template <class D>
	auto rotate_radians_with_90_multiples(const D radians, const basic_vec2& origin) {
		const auto degrees = RAD_TO_DEG<D> * radians;

		if (const auto result = rotate_by_90_multiples(degrees, origin); result != 0.f) {
			return result * DEG_TO_RAD<D>;
		}

		rotate_radians(radians, origin);
		return radians;
	}

	template <class D>
	auto& rotate(const D degrees) {
		using mult_type = std::conditional_t<std::is_floating_point_v<D>, D, real>;

		return rotate_radians(degrees * DEG_TO_RAD<mult_type>);
	}

	template <class D>
	auto& rotate(const D degrees, const basic_vec2& origin) {
		using mult_type = std::conditional_t<std::is_floating_point_v<D>, D, real>;

		return rotate_radians(degrees * DEG_TO_RAD<mult_type>, origin);
	}

	template <class T>
	basic_vec2<type>& rotate(const basic_transform<T>&); 

	template <class T>
	basic_vec2<type>& mult(const T& t) {
		rotate(t.rotation);

		x += t.pos.x;
		y += t.pos.y;
		return *this;
	}

	bool x_non_zero(const real eps = AUGS_EPSILON<real>) const {
		if constexpr(std::is_integral_v<type>) {
			return x != 0;
		}
		else {
			return repro::fabs(x) > eps;
		}
	}

	bool y_non_zero(const real eps = AUGS_EPSILON<real>) const {
		if constexpr(std::is_integral_v<type>) {
			return y != 0;
		}
		else {
			return repro::fabs(y) > eps;
		}
	}

	template <class R>
	bool is_epsilon(const R epsilon) const {
		if constexpr(std::is_integral_v<type>) {
			return x <= epsilon && y <= epsilon;
		}
		else {
			return repro::fabs(x) <= epsilon && repro::fabs(y) <= epsilon;
		}
	}

	bool is_nonzero() const {
		if constexpr(std::is_integral_v<type>) {
			return x != 0 || y != 0;
		}
		else {
			return repro::fabs(x) > AUGS_EPSILON<real> || repro::fabs(y) > AUGS_EPSILON<real>;
		}
	}

	bool neither_zero() const {
		if constexpr(std::is_integral_v<type>) {
			return x != 0 && y != 0;
		}
		else {
			return repro::fabs(x) > AUGS_EPSILON<real> && repro::fabs(y) > AUGS_EPSILON<real>;
		}
	}

	bool any_zero() const {
		if constexpr(std::is_integral_v<type>) {
			return x == 0 || y == 0;
		}
		else {
			return repro::fabs(x) < AUGS_EPSILON<real> || repro::fabs(y) < AUGS_EPSILON<real>;
		}
	}

	bool is_zero() const {
		if constexpr(std::is_integral_v<type>) {
			return x == 0 && y == 0;
		}
		else {
			return repro::fabs(x) <= AUGS_EPSILON<real> && repro::fabs(y) <= AUGS_EPSILON<real>;
		}
	}

	basic_vec2 operator-() const { return basic_vec2(x * -1, y * -1); }

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
	bool compare_abs(const basic_vec2& b, const real epsilon = AUGS_EPSILON<real>) const {
		if (repro::fabs(x - b.x) < epsilon && repro::fabs(y - b.y) < epsilon) {
			return true;
		}

		return false;
	}

	template <class A = type, class = std::enable_if_t<std::is_floating_point_v<A>>>
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

	bool to_left_of(const segment_type& s) const {
		return (*this - s[0]).cross(s[1] - s[0]) > 0.f;
	}

	bool to_right_of(const segment_type& s) const {
		return (*this - s[0]).cross(s[1] - s[0]) < 0.f;
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

	auto hash() const {
		return augs::hash_multiple(x, y);
	}
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
const basic_vec2<type> basic_vec2<type>::zero = { static_cast<type>(0), static_cast<type>(0) };

namespace std {
	template <class T>
	struct hash<basic_vec2<T>> {
		std::size_t operator()(const basic_vec2<T> v) const {
			return v.hash();
		}
	};
}

template <class S, class T>
decltype(auto) operator<<(S& out, const basic_vec2<T>& x) {
	return out << "(" << x.x << ";" << x.y << ")"; 
}