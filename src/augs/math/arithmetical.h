#pragma once
#include <type_traits>
#include "augs/misc/simple_pair.h"
#include "augs/templates/algorithm_templates.h"

template <class T>
constexpr T AUGS_EPSILON = static_cast<T>(0.0001);

template <class T>
constexpr T DEG_TO_RAD = static_cast<T>(0.01745329251994329576923690768489);

template <class T>
constexpr T RAD_TO_DEG = static_cast<T>(1.0 / 0.01745329251994329576923690768489);

template <class T>
constexpr T PI = static_cast<T>(3.1415926535897932384626433832795);

namespace augs {
	template<class T>
	T normalize_degrees(const T degrees) {
		return repro::fmod(degrees + 180, 360) - 180;
	}

	template <class T>
	auto disturbance(const T& danger_distance, const T& comfort_zone) {
		return std::max(static_cast<T>(0), static_cast<T>(1) - danger_distance / comfort_zone);
	}

	template <class T>
	bool flip_if_gt(T& value, const T& bound) {
		const auto diff = value - bound;

		if (diff > static_cast<T>(0)) {
			value -= diff;
			return true;
		}

		return false;
	}

	template <class T>
	bool flip_if_lt(T& value, const T& bound) {
		const auto diff = value - bound;

		if (diff < static_cast<T>(0)) {
			value += diff;
			return true;
		}

		return false;
	}

	template <class T>
	bool is_epsilon(const T& t, const T eps) {
		return repro::fabs(t) <= eps;
	}

	template <class T>
	bool is_zero(const T& t) {
		return repro::fabs(t) <= AUGS_EPSILON<T>;
	}

	template <class T>
	bool is_nonzero(const T& t) {
		return repro::fabs(t) > AUGS_EPSILON<T>;
	}

	template <class T>
	bool is_positive_epsilon(const T& t) {
		return t >= AUGS_EPSILON<T>;
	}

	template <class T>
	auto zigzag(const T current_time, const T cycle) {
		static_assert(std::is_floating_point_v<T>);
		const auto m = current_time / cycle;
		const auto i = static_cast<int>(m);

		return i % 2 ? (1 - (m - i)) : (m - i);
	}

	template <class T>
	auto ping_pong_2_inverse(const T current, const T cycle) {
		static_assert(std::is_integral_v<T>);

		const auto m = current / cycle;
		const auto rest = current % cycle;

		return m % 2 ? (cycle - rest - 1) : rest;
	}

	template <class T>
	auto ping_pong_2_flip(const T current, const T cycle) {
		static_assert(std::is_integral_v<T>);

		const auto m = current / cycle;
		const auto rest = current % cycle;

		return augs::simple_pair(rest, m % 2);
	}

	template <class T>
	auto ping_pong_4_flip_inverse(const T current, const T cycle) {
		static_assert(std::is_integral_v<T>);

		const auto m = current / cycle;
		const auto rest = current % cycle;

		switch (m % 4) {
			case 0: return augs::simple_pair(rest, false);
			case 1: return augs::simple_pair(cycle - rest - 1, false);
			case 2: return augs::simple_pair(rest, true);
			default: return augs::simple_pair(cycle - rest - 1, true);
		}
	}

	template <class T> 
	int sgn(const T val) {
		constexpr auto zero = static_cast<T>(0);
		return (zero < val) - (val < zero);
	}

	template <class T>
	bool compare(const T a, const T b, const T eps = AUGS_EPSILON<T>) {
		return repro::fabs(a - b) < eps;
	}

	template <class T, class A>
	T interp(const T a, const T b, const A alpha) {
		return static_cast<T>(a * (static_cast<A>(1) - alpha) + b * alpha);
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
	auto damp(const T val, const T delta, const T multiplier) {
		static_assert(std::is_floating_point_v<T>);

		constexpr auto one = static_cast<T>(1);
		return val * (one / (one + delta * multiplier));
	}

	template <class type_val>
	void shrink(type_val& val, const type_val len) {
		constexpr auto zero = static_cast<type_val>(0);

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

	template <class T>
	bool to_near_90_multiple(T& degrees) {
		const std::array<T, 5> angles = {
			static_cast<T>(0),
			static_cast<T>(90),
			static_cast<T>(-90),
			static_cast<T>(-180),
			static_cast<T>(180)
		};

		for (const auto a : angles) {
			if (augs::compare(degrees, a)) {
				degrees = a;
				return true;
			}
		}

		return false;
	}
}
