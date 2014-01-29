#pragma once
#include <random>

extern int randval(int min, int max);
extern unsigned randval(unsigned min, unsigned max);
extern float randval(float min, float max);

extern unsigned randval(std::pair<unsigned, unsigned>);
extern float randval(std::pair<float, float>);