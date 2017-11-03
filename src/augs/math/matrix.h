#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#undef near
#undef far

namespace augs {
	template <class T>
	std::array<T, 16> orthographic_projection(const T left, const T right, const T bottom, const T top, const T near, const T far) {
		return{
			2 / (right - left), 0, 0, 0,
			0, 2 / (top - bottom), 0, 0,
			0, 0, -2 / (far - near), 0,
			-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1
		};
	}

	template <class T>
	std::array<T, 16> orthographic_projection(const basic_vec2<T> visible_area) {
		return augs::orthographic_projection<T>(
			0.f,
			visible_area.x,
			visible_area.y,
			0.f,
			0.f,
			1.f
			);
	}

	template <class T>
	std::array<T, 16> orthographic_projection(const basic_ltrb<T> visible_area) {
		return augs::orthographic_projection<T>(
			visible_area.l,
			visible_area.r,
			visible_area.b,
			visible_area.t,
			0.f,
			1.f
		);
	}
}
