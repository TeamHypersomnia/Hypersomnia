#pragma once
#include "augs/math/vec2.h"

template <class motion_type_enum, class motion_offset>
struct basic_input_motion {
	motion_type_enum motion = motion_type_enum::INVALID;
	motion_offset offset;

	bool is_set() const {
		return motion != motion_type_enum::INVALID;
	}

	bool operator==(const basic_input_motion& b) const {
		return
			motion == b.motion
			&& offset == b.offset
		;
	}

	bool operator!=(const basic_input_motion& b) const {
		return !operator==(b);
	}
};