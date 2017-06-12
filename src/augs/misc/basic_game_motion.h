#pragma once
#include "augs/math/vec2.h"

using controller_motion = vec2t<short>;

template <class motion_type_enum>
struct basic_game_motion {
	motion_type_enum motion = motion_type_enum::INVALID;
	controller_motion offset;

	void set_motion_type(const motion_type_enum t) {
		motion = t;
	}

	auto get_motion_type() const {
		return motion;
	}

	bool is_set() const {
		return motion != motion_type_enum::INVALID;
	}

	bool operator==(const basic_game_motion& b) const {
		return
			motion == b.motion
			&& offset == b.offset
		;
	}

	bool operator!=(const basic_game_motion& b) const {
		return !operator==(b);
	}
};