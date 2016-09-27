#include "transform_component.h"
#include "physics_component.h"
#include "Box2D/Common/b2Math.h"

namespace augs {

}

namespace components {

	transform::transform(float x, float y, float rotation) : pos(vec2(x, y)), rotation(rotation) {}
	transform::transform(vec2 pos, float rotation) : pos(pos), rotation(rotation) {}

	transform transform::operator+(const transform& b) const {
		transform out;
		out.pos = pos + b.pos;
		out.rotation = rotation + b.rotation;
		return out;
	}

	transform transform::to_si_space() const {
		return{ pos * PIXELS_TO_METERSf, rotation * DEG_TO_RADf };
	}

	transform transform::to_user_space() const {
		return{ pos * METERS_TO_PIXELSf, rotation * RAD_TO_DEGf };
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

	transform transform::interpolated(const transform& previous, float ratio, float epsilon) const {
		auto result = *this;
		auto interpolated = augs::interp(previous, *this, ratio);

		if ((pos - interpolated.pos).length_sq() > epsilon)
			result.pos = interpolated.pos;
		if (std::abs(rotation - interpolated.rotation) > epsilon)
			result.rotation = interpolated.rotation;

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
}