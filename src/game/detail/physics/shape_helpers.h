#pragma once
#include <array>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>

template <class C>
auto to_polygon_shape(const C& arr) {
	b2PolygonShape output;
	output.Set(arr.data(), static_cast<int32>(arr.size()));
	return output;
}

template <class C>
auto to_polygon_shape(C arr, const si_scaling si) {
	for (auto& a : arr) {
		a = si.get_meters(a);
	}

	return to_polygon_shape(arr);
}
