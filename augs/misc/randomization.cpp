#include "randomization.h"

randomization::randomization(size_t seed) {
	generator.seed(seed);
}

int randomization::randval(int min, int max) {
	if (min == max) return min;
	return std::uniform_int_distribution<int>(min, max)(generator);
}

unsigned randomization::randval(unsigned min, unsigned max) {
	if (min == max) return min;
	return std::uniform_int_distribution<unsigned>(min, max)(generator);
}

float randomization::randval(float min, float max) {
	if (min == max) return min;
	return std::uniform_real_distribution<float>(min, max)(generator);
}

float randomization::randval(float minmax) {
	return randval(-minmax, minmax);
}

unsigned randomization::randval(std::pair<unsigned, unsigned> p) {
	return randval(p.first, p.second);
}

float randomization::randval(std::pair<float, float> p) {
	return randval(p.first, p.second);
}