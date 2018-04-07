#include "animation.h"

void animation::create_frames(
	const assets::image_id first_frame,
	const assets::image_id last_frame,
	const float frame_duration_ms
) {
	for (assets::image_id i = first_frame; i < last_frame; i = assets::image_id(int(i) + 1)) {
		animation_frame frame;
		frame.duration_milliseconds = frame_duration_ms;
		frame.image_id = i;

		frames.push_back(frame);
	}
}