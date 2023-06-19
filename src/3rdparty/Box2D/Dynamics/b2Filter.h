#pragma once
#include <cstdint>

/// This holds contact filtering data.
struct b2Filter
{
	b2Filter()
	{
		categoryBits = 0x0001;
		maskBits = 0xFFFF;
		groupIndex = 0;
	}

	// GEN INTROSPECTOR struct b2Filter
	uint16_t categoryBits;
	uint16_t maskBits;
	int16_t groupIndex;
	// END GEN INTROSPECTOR

	bool operator==(const b2Filter& b) const {
		return 
			categoryBits == b.categoryBits 
			&& maskBits == b.maskBits
			&& groupIndex == b.groupIndex
		;
	}

	static auto nothing() {
		b2Filter out;
		out.categoryBits = 0;
		out.maskBits = 0;
		out.groupIndex = 0;

		return out;
	}

	bool operator!=(const b2Filter& b) const {
		return !operator==(b);
	}
};
