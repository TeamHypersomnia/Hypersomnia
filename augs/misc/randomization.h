#pragma once
#include <random>
#include <utility>

struct randomization {
	std::mt19937 deterministic_generator;
	std::mt19937 drawing_time_generator;
	std::mt19937* current_generator = nullptr;
public:
	randomization();

	void enable_drawing_time_random_generator();
	void enable_deterministic_random_generator();
	std::mt19937& get_current_generator();

	int randval(int min, int max);
	unsigned randval(unsigned min, unsigned max);
	float randval(float min, float max);

	unsigned randval(std::pair<unsigned, unsigned>);
	float randval(std::pair<float, float>);
};
