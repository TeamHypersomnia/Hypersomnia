#pragma once
#include <type_traits>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"

template <class T>
struct basic_transform {
	using transform = basic_transform<T>;
	using vec2 = basic_vec2<T>;

	// GEN INTROSPECTOR struct basic_transform class T
	vec2 pos;
	T rotation;
	// END GEN INTROSPECTOR

	basic_transform(
		const T x = static_cast<T>(0),
		const T y = static_cast<T>(0),
		const T rotation = static_cast<T>(0)
	) :
		pos(vec2(x, y)),
		rotation(rotation)
	{}

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

	template <
		class b2_transform_type, 
		class b2_sweep_type, 
		class K = T,
		class = std::enable_if_t<std::is_same_v<K, real32>>
	>
	void to_box2d_transforms(
		b2_transform_type& m_xf,
		b2_sweep_type& m_sweep
	) const {
		m_xf.p.x = pos.x;
		m_xf.p.y = pos.y;
		m_xf.q.Set(rotation);

		m_sweep.localCenter.SetZero();
		m_sweep.c0 = m_xf.p;
		m_sweep.c = m_xf.p;
		m_sweep.a0 = rotation;
		m_sweep.a = rotation;
		m_sweep.alpha0 = 0.0f;
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
		return pos.compare(b.pos) && std::abs(rotation - b.rotation) <= rotational_eps;
	}
};

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