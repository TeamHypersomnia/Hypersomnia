#include "augs/audio/sound_data.h"
#include "augs/ensure.h"

namespace augs {
	std::vector<int16_t> mix_stereo_to_mono(const std::vector<sound_sample_type>& samples) {
		ensure(samples.size() % 2 == 0);

		std::vector<sound_sample_type> output;
		output.resize(samples.size() / 2);

		for (size_t i = 0; i < samples.size(); i += 2) {
			output.at(i/2) = (static_cast<int>(samples.at(i)) + samples.at(i+1)) / 2;
		}

		return output;
	}

	sound_data mix_stereo_to_mono(const sound_data& source) {
		sound_data mono_data;
		mono_data.channels = 1;
		mono_data.frequency = source.frequency;
		mono_data.samples = mix_stereo_to_mono(source.samples);
		return mono_data;
	}
}