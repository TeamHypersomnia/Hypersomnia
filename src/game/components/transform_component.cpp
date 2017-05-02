#include "transform_component.h"
#include "rigid_body_component.h"
#include "Box2D/Common/b2Math.h"
#include "augs/math/si_scaling.h"

namespace components {
	transform::transform(
		const float x, 
		const float y, 
		const float rotation
	) : 
		pos(vec2(x, y)), 
		rotation(rotation) 
	{}

	transform::transform(
		const vec2 pos, 
		const float rotation
	) : 
		pos(pos), 
		rotation(rotation) 
	{}

	transform transform::operator*(const transform& offset) const {
		return {
			pos + vec2(offset.pos).rotate(rotation, vec2()),
			rotation + offset.rotation
		};
	}

	transform transform::operator+(const transform& b) const {
		transform out;
		out.pos = pos + b.pos;
		out.rotation = rotation + b.rotation;
		return out;
	}

	transform transform::to_si_space(const si_scaling si) const {
		return{ si.get_meters(pos), rotation * DEG_TO_RAD<float> };
	}

	transform transform::to_user_space(const si_scaling si) const {
		return{ si.get_pixels(pos), rotation * RAD_TO_DEG<float> };
	}

	transform transform::operator-(const transform& b) const {
		transform out;
		out.pos = pos - b.pos;
		out.rotation = rotation - b.rotation;
		return out;
	}

	transform& transform::operator+=(const transform& b) {
		(*this) = (*this) + b;
		return *this;
	}

	bool transform::operator==(const transform& b) const {
		return pos == b.pos && rotation == b.rotation;
	}

	void transform::to_box2d_transforms(b2Transform& m_xf, b2Sweep& m_sweep) const {
		m_xf.p = pos;
		m_xf.q.Set(rotation);

		m_sweep.localCenter.SetZero();
		m_sweep.c0 = m_xf.p;
		m_sweep.c = m_xf.p;
		m_sweep.a0 = rotation;
		m_sweep.a = rotation;
		m_sweep.alpha0 = 0.0f;
	}

	transform transform::interpolated(const transform& previous, const float ratio, const float epsilon) const {
		auto result = *this;
		auto interpolated = augs::interp(previous, *this, ratio);

		if ((pos - interpolated.pos).length_sq() > epsilon) {
			result.pos = interpolated.pos;
		}
		if (std::abs(rotation - interpolated.rotation) > epsilon) {
			result.rotation = interpolated.rotation;
		}

		return result;
	}

	transform transform::interpolated_separate(const transform& previous, const float positional_ratio, const float rotational_ratio, const float epsilon) const {
		auto result = *this;
		const auto& a = previous;
		const auto& b = *this;

		components::transform interpolated;
		interpolated.pos = augs::interp(a.pos, b.pos, positional_ratio);
		interpolated.rotation = augs::interp(vec2().set_from_degrees(a.rotation), vec2().set_from_degrees(b.rotation), rotational_ratio).degrees();

		if ((pos - interpolated.pos).length_sq() > epsilon) {
			result.pos = interpolated.pos;
		}
		if (std::abs(rotation - interpolated.rotation) > epsilon) {
			result.rotation = interpolated.rotation;
		}

		return result;
	}

	void transform::flip_rotation() {
		rotation = -rotation;
	}

	void transform::reset() {
		pos.reset();
		rotation = 0.f;
	}

	vec2 transform::interpolation_direction(const transform& previous) const {
		return pos - previous.pos;
	}

	vec2 transform::get_orientation() const {
		return vec2().set_from_degrees(rotation);
	}
}