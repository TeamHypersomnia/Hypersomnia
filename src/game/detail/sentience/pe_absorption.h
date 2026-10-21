#pragma once
#include <algorithm>

struct pe_absorption_info {
	// GEN INTROSPECTOR struct pe_absorption_info
	real32 hp = 0.f;
	real32 cp = 0.f;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return hp != 0.f && cp != 0.f;
	}

	/*
		How much Personal Electricity one point of health/consciousness damage costs - never quite free.
	*/

	real32 get_hp_mult() const {
		return std::max(0.01f, hp);
	}

	real32 get_cp_mult() const {
		return std::max(0.01f, cp);
	}
};

