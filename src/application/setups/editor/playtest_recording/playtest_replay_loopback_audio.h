#pragma once
#include <cstdint>
#include <vector>
#include "augs/audio/audio_context.h"
#include "application/setups/editor/playtest_recording/playtest_replay_capture_settings.h"

class playtest_replay_loopback_audio {
	std::vector<int16_t> samples;

public:
	const std::vector<int16_t>& render_one_render(augs::audio_context& audio) {
		const auto num_sample_frames = static_cast<int>(playtest_replay_capture_audio_sample_frames_per_render_v);

		samples.resize(num_sample_frames * 2);
		audio.render_loopback_stereo(samples.data(), num_sample_frames);

		return samples;
	}
};
