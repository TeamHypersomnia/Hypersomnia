#pragma once
#include <type_traits>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"

struct b2Transform;

template <class T>
const std::string& get_type_name();

template <class T>
struct basic_transform {
	using transform = basic_transform<T>;
	using vec2 = basic_vec2<T>;
	using R = real32;

	static std::string get_custom_type_name() {
		return "transform_" + get_type_name<T>();
	}

	static constexpr bool reinfer_when_tweaking = true;

	// GEN INTROSPECTOR struct basic_transform class T
	vec2 pos;
	R rotation = static_cast<R>(0);
	// END GEN INTROSPECTOR

	basic_transform() = default;

	basic_transform(
		const vec2 pos,
		const R rotation = static_cast<R>(0)
	) :
		pos(pos),
		rotation(rotation)
	{}

	template <class V, class = std::enable_if_t<std::is_same_v<V, b2Transform>>>
	explicit operator V() const {
		if constexpr(std::is_same_v<V, b2Transform>) {
			V t;
			t.p.x = pos.x;
			t.p.y = pos.y;
			t.q.Set(rotation);
			return t;
		}
		else {
			static_assert(always_false_v<V>);
		}
	}

	template <class V>
	auto to(const si_scaling si) const {
		auto si_space = to_si_space(si);
		return si_space.operator V();
	}

	transform operator*(const transform& offset) const {
		return {
			pos + vec2(offset.pos).rotate(rotation),
			rotation + offset.rotation
		};
	}

	transform& operator*=(const transform& offset) {
		pos += vec2(offset.pos).rotate(rotation); 
		rotation += offset.rotation;
		return *this;
	}

	transform operator+(const transform& b) const {
		transform out;
		out.pos = pos + b.pos;
		out.rotation = rotation + b.rotation;
		return out;
	}

	transform to_si_space(const si_scaling si) const {
		return{ si.get_meters(pos), rotation * DEG_TO_RAD<R> };
	}

	transform to_user_space(const si_scaling si) const {
		return{ si.get_pixels(pos), rotation * RAD_TO_DEG<R> };
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

	bool operator!=(const transform& b) const {
		return !operator==(b);
	}

	auto interp(
		const transform next,
		const R alpha
	) const {
		return transform{
			augs::interp(pos, next.pos, alpha),
			augs::interp(get_direction(), next.get_direction(), alpha).degrees()
		};
	}

	template <class A, class B>
	auto interp_separate(
		const transform next,
		const A positional_alpha,
		const B rotational_alpha
	) const {
		return transform{
			augs::interp(pos, next.pos, positional_alpha),
			augs::interp(get_direction(), next.get_direction(), rotational_alpha).degrees()
		};
	}

	template <class E>
	auto& snap_to(
		const transform to,
		const E epsilon
	) {
		if ((pos - to.pos).length_sq() > epsilon) {
			pos = to.pos;
		}

		if (repro::fabs(rotation - to.rotation) > epsilon) {
			rotation = to.rotation;
		}

		return *this;
	}

	template <class A, class B>
	auto& snap_to(
		const transform to,
		const A positional_epsilon,
		const B rotational_epsilon
	) {
		if ((pos - to.pos).length_sq() > positional_epsilon) {
			pos = to.pos;
		}

		if (repro::fabs(rotation - to.rotation) > rotational_epsilon) {
			rotation = to.rotation;
		}
		
		return *this;
	}

	auto& flip_rotation() {
		rotation = -rotation;
		return *this;
	}

	void flip_vertically() {
		pos.y = -pos.y;
		rotation = -rotation;
	}

	void reset() {
		pos.reset();
		rotation = static_cast<R>(0);
	}

	auto interpolation_direction(const transform& previous) const {
		return pos - previous.pos;
	}

	auto get_direction() const {
		return vec2::from_degrees(rotation);
	}

	template <class A = real32, class B = real32>
	bool compare(
		const transform& b,
		const A positional_eps = AUGS_EPSILON<A>,
		const B rotational_eps = AUGS_EPSILON<B>
	) const {
		return pos.compare_abs(b.pos, positional_eps) && repro::fabs(rotation - b.rotation) <= rotational_eps;
	}

	bool negliglible() const {
		return compare({});
	}

	auto& rotate(const R degrees, const vec2 origin) {
		pos.rotate(degrees, origin);
		rotation += degrees;
		return *this;
	}

	auto& rotate_radians(const R radians, const vec2 origin) {
		pos.rotate_radians(radians, origin);
		rotation += radians;
		return *this;
	}

	auto rotate_degrees_with_90_multiples(const R degrees, const vec2 origin) {
		const auto delta = pos.rotate_degrees_with_90_multiples(degrees, origin);
		rotation += delta;
		return delta;
	}

	auto rotate_radians_with_90_multiples(const R radians, const vec2 origin) {
		const auto delta = pos.rotate_radians_with_90_multiples(radians, origin);
		rotation += delta;
		return delta;
	}

	basic_transform<int> get_integerized() const {
		return {
			static_cast<vec2i>(pos),
			rotation
		};
	}
};

template <class T>
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

template <class type>
template <class T>
basic_vec2<type>& basic_vec2<type>::rotate(const basic_transform<T>& t) {
	return rotate(t.rotation, t.pos);
}
