#pragma once
#include <random>

extern int randval(int min, int max);
extern unsigned randval(unsigned min, unsigned max);
extern float randval(float min, float max);

extern unsigned randval(std::pair<unsigned, unsigned>);
extern float randval(std::pair<float, float>);


extern int randval(int min, int max, std::mt19937& gen);
extern unsigned randval(unsigned min, unsigned max, std::mt19937& gen);
extern float randval(float min, float max, std::mt19937& gen);

extern unsigned randval(std::pair<unsigned, unsigned>, std::mt19937& gen);
extern float randval(std::pair<float, float>, std::mt19937& gen);