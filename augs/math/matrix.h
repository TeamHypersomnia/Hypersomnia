#pragma once
#include <vector>

#undef near
#undef far

namespace augs {
	template <class T>
	std::vector<T> orthographic_projection(T left, T right, T bottom, T top, T near, T far) {
		return{
			2 / (right - left), 0, 0, 0,
			0, 2 / (top - bottom), 0, 0,
			0, 0, -2 / (far - near), 0,
			-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1
		};
	}
}
