#pragma once
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <vector>
#include "augs/filesystem/path.h"
#include "application/setups/editor/playtest_recording/playtest_capture_output_file.h"

class playtest_replay_capture_output {
	playtest_capture_output_file frames_output;
	std::optional<playtest_capture_output_file> audio_output;

	std::mutex audio_of_frames_lock;
	std::deque<std::vector<int16_t>> audio_of_frames_not_yet_written;

	uint32_t num_requested = 0;
	uint32_t num_written = 0;
	bool last_requested = false;

	void write_audio_of_oldest_frame() {
		auto audio_of_frame = std::vector<int16_t>();

		{
			auto lock = std::scoped_lock(audio_of_frames_lock);

			if (audio_of_frames_not_yet_written.empty()) {
				return;
			}

			audio_of_frame = std::move(audio_of_frames_not_yet_written.front());
			audio_of_frames_not_yet_written.pop_front();
		}

		audio_output->write(audio_of_frame.data(), audio_of_frame.size() * sizeof(int16_t));
	}

public:
	playtest_replay_capture_output(
		const augs::path_type& raw_rgba_frames_path,
		const augs::path_type& raw_s16le_stereo_audio_path
	) :
		frames_output(raw_rgba_frames_path)
	{
		if (!raw_s16le_stereo_audio_path.empty()) {
			audio_output.emplace(raw_s16le_stereo_audio_path);
		}
	}

	bool wants_more_frames() const {
		return !last_requested;
	}

	void on_frame_requested(const bool is_last) {
		++num_requested;
		last_requested = is_last;
	}

	void queue_audio_of_requested_frame(const std::vector<int16_t>& samples) {
		if (audio_output.has_value()) {
			auto lock = std::scoped_lock(audio_of_frames_lock);
			audio_of_frames_not_yet_written.push_back(samples);
		}
	}

	template <class Image>
	void write_frame(const Image& frame) {
		frames_output.write(frame.data(), static_cast<std::size_t>(frame.get_size().area() * 4));

		if (audio_output.has_value()) {
			write_audio_of_oldest_frame();
		}

		++num_written;
	}

	bool complete() const {
		return last_requested && num_written >= num_requested;
	}
};
