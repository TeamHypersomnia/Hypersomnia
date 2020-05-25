#pragma once
#include "game/components/sorting_order_component.h"

enum class test_ground_order : sorting_order_type {
	SOIL,
	FLOOR_AND_ROAD,
	ON_FLOOR,
	ON_ON_FLOOR,

	AQUARIUM_FLOWERS,
	AQUARIUM_DUNES,

	BOTTOM_FISH,
	UPPER_FISH,

	WATER_COLOR_OVERLAYS,
	WATER_SURFACES
};

template <class T>
render_layer get_layer_for_order_type(const T t) {
	if constexpr(std::is_same_v<T, render_layer>) {
		return t;
	}
	else if constexpr(std::is_same_v<T, test_ground_order>) {
		return render_layer::GROUND;
	}

	return render_layer::INVALID;
}
