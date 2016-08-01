#pragma once 
#include "augs/math/vec2.h"
#include "game/components/transform_component.h"
#include "game/components/fixtures_component.h"

#include <cmath>

template<class Container>
components::transform calculate_intermediate_transform(const Container& offset_pattern, const double factor) {
	components::transform result = offset_pattern[offset_pattern.size() - 1];
	double distance = 0;

	for (size_t i = 1; i < offset_pattern.size(); ++i)
		distance += vec2(offset_pattern[i].pos - offset_pattern[i - 1].pos).length();

	double position = distance * factor;

	for (size_t i = 1; i < offset_pattern.size(); ++i) {
		if (position <= vec2(offset_pattern[i].pos - offset_pattern[i - 1].pos).length()) {
			double alpha = position / vec2(offset_pattern[i].pos - offset_pattern[i - 1].pos).length();
			result = augs::interp(offset_pattern[i], offset_pattern[i - 1], alpha);
			return result;
		}
		else {
			position -= vec2(offset_pattern[i].pos - offset_pattern[i - 1].pos).length();
		}
	}
	return result;
}