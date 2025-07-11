#pragma once
#include <cstdint>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/templates/exception_templates.h"

namespace augs {
	using sound_sample_type = int16_t;

	struct sound_decoding_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	struct sound_data {
		std::vector<sound_sample_type> samples;
		int frequency = 0;
		int channels = 0;

		sound_data(const path_type& path);

		double compute_length_in_seconds() const;
	};
}