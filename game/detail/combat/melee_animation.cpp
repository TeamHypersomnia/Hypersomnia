#include "melee_animation.h"

components::transform melee_animation::calculate_intermediate_transform(double factor) const {
	components::transform result = offset_pattern[offset_pattern.size() - 1];
	double distance = 0;

	for (int i = 1; i < offset_pattern.size(); ++i)
		distance += vec2(offset_pattern[i].pos - offset_pattern[i - 1].pos).length();

	double position = distance * factor;

	for (int i = 1;i < offset_pattern.size();++i) {
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