#include "randomization.h"

randomization::randomization() {
	deterministic_generator.seed(0);
	drawing_time_generator.seed(0);
}

void randomization::enable_drawing_time_random_generator() {
	current_generator = &drawing_time_generator;
}

void randomization::enable_deterministic_random_generator() {
	current_generator = &deterministic_generator;
}

std::mt19937& randomization::get_current_generator() {
	return *current_generator;
}

int randomization::randval(int min, int max) {
	if (min == max) return min;
	return std::uniform_int_distribution<int>(min, max)(get_current_generator());
}

unsigned randomization::randval(unsigned min, unsigned max) {
	if (min == max) return min;
	return std::uniform_int_distribution<unsigned>(min, max)(get_current_generator());
}

float randomization::randval(float min, float max) {
	if (min == max) return min;
	return std::uniform_real_distribution<float>(min, max)(get_current_generator());
}

unsigned randomization::randval(std::pair<unsigned, unsigned> p) {
	return randval(p.first, p.second);
}

float randomization::randval(std::pair<float, float> p) {
	return randval(p.first, p.second);
}