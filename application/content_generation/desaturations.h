#pragma once
#include <vector>
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include <experimental/filesystem>

struct desaturation_metadata {
	std::experimental::filesystem::file_time_type last_write_time_of_source;
};

namespace augs {
	template <class A>
	bool read_object(A& ar, desaturation_metadata& data) {
		return
			read_object(ar, data.last_write_time_of_source)
			;
	}

	template <class A>
	void write_object(A& ar, const desaturation_metadata& data) {
		write_object(ar, data.last_write_time_of_source);
	}
}

void regenerate_desaturations();