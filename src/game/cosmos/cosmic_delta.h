#pragma once
#include "augs/readwrite/memory_stream.h"

class cosmos;

class cosmic_delta {
public:
	static bool encode(const cosmos& base, const cosmos& encoded, augs::memory_stream& to);
	static void decode(cosmos& into, augs::memory_stream& from, const bool reinfer_partially = false);
};