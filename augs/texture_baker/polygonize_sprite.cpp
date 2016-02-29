#include "image.h"
#include "log.h"

namespace augs {
	std::vector<vec2i> image::get_polygonized() const {
		std::vector<vec2i> sample;
		sample.push_back(vec2i(0, 0));
		sample.push_back(vec2i(size.w, 0));
		sample.push_back(vec2i(size.w, size.h-10));
		sample.push_back(vec2i(0, size.h));
		return sample;
	}
}

