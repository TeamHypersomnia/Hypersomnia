#pragma once
#include "augs/readwrite/streams.h"

class cosmos;

class cosmic_delta {
public:
	static bool encode(const cosmos& base, const cosmos& encoded, augs::stream& to);
	static void decode(cosmos& into, augs::stream& from, const bool reinfer_partially = false);
};