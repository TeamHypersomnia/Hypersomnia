#pragma once
#include <array>

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
}
