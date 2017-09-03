#pragma once
#include "augs/misc/enum_boolset.h"

enum class flip {
	HORIZONTALLY,
	VERTICALLY,

	COUNT
};

struct flip_flags : augs::enum_boolset<flip, 1> {
	// GEN INTROSPECTOR struct flip_flags
	// INTROSPECT BASE augs::enum_boolset<flip, 1>
	// END GEN INTROSPECTOR
	using augs::enum_boolset<flip, 1>::enum_boolset;

	bool horizontally() const { 
		return test(flip::HORIZONTALLY); 
	}

	bool vertically() const { 
		return test(flip::VERTICALLY); 
	}
};