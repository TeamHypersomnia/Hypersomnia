#pragma once
#include <type_traits>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"

template <class T>
struct basic_transform {
	using transform = basic_transform<T>;
	using vec2 = basic_vec2<T>;

	static constexpr bool reinfer_when_tweaking = true;

	// GEN INTROSPECTOR struct basic_transform class T
	vec2 pos;
	T rotation = static_cast<T>(0);
	// END GEN INTROSPECTOR

	basic_transform() = default;

	basic_transform(
		const vec2 pos,
		const T rotation = static_cast<T>(0)
	) :
		pos(pos),
		rotation(rotation)
	{}

	transform operator*(const transform& offset) const {
		return {
			pos + vec2(offset.pos).rotate(rotation, vec2()),
			rotation + offset.rotation
		};
	}

	transform operator+(const transform& b) const {
		transform out;
		out.pos = pos + b.pos;
		out.rotation = rotation + b.rotation;
		return out;
	}

	transform to_si_space(const si_scaling si) const {
		return{ si.get_meters(pos), rotation * DEG_TO_RAD<T> };
	}

	transform to_user_space(const si_scaling si) const {
		return{ si.get_pixels(pos), rotation * RAD_TO_DEG<T> };
	}

	transform operator-(const transform& b) const {
		transform out;
		out.pos = pos - b.pos;
		out.rotation = rotation - b.rotation;
		return out;
	}

	transform& operator+=(const transform& b) {
		(*this) = (*this) + b;
		return *this;
	}

	bool operator==(const transform& b) const {
		return pos == b.pos && rotation == b.rotation;
	}

	auto interp(
		const transform next,
		const T alpha
	) const {
		return transform{
			augs::interp(pos, next.pos, alpha),
			augs::interp(vec2::from_degrees(rotation), vec2::from_degrees(next.rotation), alpha).degrees()
		};
	}

	auto interp_separate(
		const transform next,
		const T positional_alpha,
		const T rotational_alpha
	) const {
		return transform{
			augs::interp(pos, next.pos, positional_alpha),
			augs::interp(vec2::from_degrees(rotation), vec2::from_degrees(next.rotation), rotational_alpha).degrees()
		};
	}

	auto& snap_to(
		const transform to,
		const T epsilon
	) {
		if ((pos - to.pos).length_sq() > epsilon) {
			pos = to.pos;
		}

		if (std::abs(rotation - to.rotation) > epsilon) {
			rotation = to.rotation;
		}

		return *this;
	}

	auto& snap_to(
		const transform to,
		const T positional_epsilon,
		const T rotational_epsilon
	) {
		if ((pos - to.pos).length_sq() > positional_epsilon) {
			pos = to.pos;
		}

		if (std::abs(rotation - to.rotation) > rotational_epsilon) {
			rotation = to.rotation;
		}
		
		return *this;
	}

	auto& flip_rotation() {
		rotation = -rotation;
		return *this;
	}

	void reset() {
		pos.reset();
		rotation = static_cast<T>(0);
	}

	auto interpolation_direction(const transform& previous) const {
		return pos - previous.pos;
	}

	auto get_orientation() const {
		return vec2::from_degrees(rotation);
	}

	bool compare(
		const transform& b,
		const T positional_eps = AUGS_EPSILON<T>,
		const T rotational_eps = AUGS_EPSILON<T>
	) const {
		return pos.compare_abs(b.pos, positional_eps) && std::abs(rotation - b.rotation) <= rotational_eps;
	}

	bool negliglible() const {
		return compare({});
	}

	auto& rotate(const T degrees, const vec2 origin) {
		pos.rotate(degrees, origin);
		rotation += degrees;
		return *this;
	}

	auto& rotate_radians(const T radians, const vec2 origin) {
		pos.rotate_radians(radians, origin);
		rotation += radians;
		return *this;
	}

	auto& rotate_degrees_with_90_multiples(const T degrees, const vec2 origin) {
		rotation += augs::rotate_degrees_with_90_multiples(pos, origin, degrees);
		return *this;
	}

	auto& rotate_radians_with_90_multiples(const T radians, const vec2 origin) {
		rotation += augs::rotate_radians_with_90_multiples(pos, origin, radians);
		return *this;
	}
};

template<class T>
std::ostream& operator<<(std::ostream& out, const basic_transform<T>& x) {
	return out << typesafe_sprintf("(%x;%x;%x*)", x.pos.x, x.pos.y, x.rotation);
}

namespace augs {
	template <class T>
	basic_transform<T> get_relative_offset(
		const basic_transform<T> chased_origin, 
		const basic_transform<T> chasing_point 
	) {
		auto offset = chasing_point - chased_origin;
		offset.pos.rotate(-chased_origin.rotation, basic_vec2<T>::zero);
		return offset;
	}
}

namespace augs {
	template <class T>
	auto interp(
		const basic_transform<T> a,
		const basic_transform<T> b,
		const T alpha
	) {
		return a.interp(b, alpha);
	}
}

namespace std {
	template <class T>
	struct hash<basic_transform<T>> {
		std::size_t operator()(const basic_transform<T> t) const {
			return augs::simple_two_hash(t.pos, t.rotation);
		}
	};
}