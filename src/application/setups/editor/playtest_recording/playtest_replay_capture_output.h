#pragma once
#include <cstdint>
#include <fstream>
#include <optional>
#include <vector>
#include "augs/filesystem/path.h"

class playtest_replay_capture_output {
	std::ofstream frames_output;
	std::optional<std::ofstream> audio_output;

	uint32_t num_requested = 0;
	uint32_t num_written = 0;
	bool last_requested = false;

public:
	playtest_replay_capture_output(
		const augs::path_type& raw_rgba_frames_path,
		const augs::path_type& raw_s16le_stereo_audio_path
	) :
		frames_output(raw_rgba_frames_path, std::ios::out | std::ios::binary)
	{
		if (!raw_s16le_stereo_audio_path.empty()) {
			audio_output.emplace(raw_s16le_stereo_audio_path, std::ios::out | std::ios::binary);
		}
	}

	bool wants_more_frames() const {
		return !last_requested;
	}

	void on_frame_requested(const bool is_last) {
		++num_requested;
		last_requested = is_last;
	}

	void write_audio(const std::vector<int16_t>& samples) {
		if (audio_output.has_value()) {
			audio_output->write(reinterpret_cast<const char*>(samples.data()), static_cast<std::streamsize>(samples.size() * sizeof(int16_t)));
		}
	}

	template <class Image>
	void write_frame(const Image& frame) {
		frames_output.write(reinterpret_cast<const char*>(frame.data()), static_cast<std::streamsize>(frame.get_size().area() * 4));
		frames_output.flush();
		++num_written;
	}

	bool cannot_write() const {
		const bool frames_cannot_be_written = !frames_output.good();
		const bool audio_cannot_be_written = audio_output.has_value() && !audio_output->good();

		return frames_cannot_be_written || audio_cannot_be_written;
	}

	bool complete() const {
		return cannot_write() || (last_requested && num_written >= num_requested);
	}
};
