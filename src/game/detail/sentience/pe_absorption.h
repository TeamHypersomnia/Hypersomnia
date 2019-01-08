#pragma once

struct pe_absorption_info {
	// GEN INTROSPECTOR struct pe_absorption_info
	real32 hp = 0.f;
	real32 cp = 0.f;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return hp != 0.f && cp != 0.f;
	}
};

