#pragma once
#include <vector>

namespace augs {
	typedef int16_t sound_sample_type;

	struct sound_data {
		std::vector<sound_sample_type> samples;
		int frequency = 0;
		int channels = 0;

		double compute_length_in_seconds() const {
			return static_cast<double>(samples.size()) / (frequency * channels);
		}
	};

	std::vector<sound_sample_type> mix_stereo_to_mono(const std::vector<sound_sample_type>&);
	sound_data mix_stereo_to_mono(const sound_data& source);
}