#include "melee_animation.h"

melee_animation::melee_animation(std::vector<vec2> pattern) {
	offset_pattern = pattern;
}

vec2 melee_animation::update(double factor) {
	vec2 result = offset_pattern[offset_pattern.size() - 1];
	double distance = 0;
	std::vector<double> distance_vector;
	for (int i = 1; i < offset_pattern.size(); ++i) {
		distance_vector.push_back(vec2(offset_pattern[i] - offset_pattern[i - 1]).length());
		distance += distance_vector[distance_vector.size() - 1];
	}
	double position = distance * factor;
	for (int i = 0;i < distance_vector.size();++i)
	{
		if (position <= distance_vector[i]) {
			double alpha = position / distance_vector[i];
			result = augs::interp(offset_pattern[i], offset_pattern[i + 1], alpha);
			return result;
		}
		else {
			position -= distance_vector[i];
		}
	}
	return result;
}