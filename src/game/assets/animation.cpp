#include "animation.h"

bool simple_animation_state::advance(
	const real32 dt,
	const plain_animation_frames_type& source,
	const unsigned frame_offset
) {
	advance({ dt, static_cast<unsigned>(source.size()) - frame_offset }, [&](const auto i) { 
		return source[i].duration_milliseconds; 
	});

	return frame_offset + frame_num >= source.size();
}
