#include "animation.h"

bool simple_animation_state::advance(
	const real32 dt,
	const animation_frames_type& source,
	const unsigned frame_offset = 0
) {
	advance(in, [&](const auto i) { 
		return source[i].duration_milliseconds; 
	});

	return frame_offset + frame_num >= source.size();
}
