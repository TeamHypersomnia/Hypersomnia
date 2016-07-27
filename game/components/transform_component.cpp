#include "transform_component.h"

namespace augs {

}

namespace components {

	transform::transform(float x, float y, float rotation) : pos(vec2(x, y)), rotation(rotation) {}
	transform::transform(vec2 pos, float rotation) : pos(pos), rotation(rotation) {}
	transform::transform(transform::previous_state state) : transform(state.pos, state.rotation) {}

	transform transform::operator+(const transform& b) const {
		transform out;
		out.pos = pos + b.pos;
		out.rotation = rotation + b.rotation;
		return out;
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

	transform transform::interpolated(float ratio, float epsilon) const {
		auto result = *this;
		auto interpolated = augs::interp(components::transform(previous), *this, ratio);

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

	vec2 transform::interpolation_direction() const {
		return pos - previous.pos;
	}
}