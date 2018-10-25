#pragma once
#include <array>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>

template <std::size_t I>
auto to_polygon_shape(const std::array<vec2, I>& arr) {
	b2PolygonShape output;
	output.Set(arr.data(), static_cast<int32>(arr.size()));
	return output;
}

template <std::size_t I>
auto to_polygon_shape(std::array<vec2, I> arr, const si_scaling si) {
	for (auto& a : arr) {
		a = si.get_meters(a);
	}

	return to_polygon_shape(arr);
}
