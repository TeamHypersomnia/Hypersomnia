#include "randval.h"

int randval(int min, int max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_int_distribution<int>(min, max)(generator);
}

unsigned randval(unsigned min, unsigned max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_int_distribution<unsigned>(min, max)(generator);
}

float randval(float min, float max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_real_distribution<float>(min, max)(generator);
}

unsigned randval(std::pair<unsigned, unsigned> p) {
	return randval(p.first, p.second);
}

float randval(std::pair<float, float> p) {
	return randval(p.first, p.second);
}