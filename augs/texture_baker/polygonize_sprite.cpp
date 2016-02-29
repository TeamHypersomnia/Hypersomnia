#include "image.h"
#include "log.h"

namespace augs {
	std::vector<vec2i> image::get_polygonized() const {
		std::vector<vec2i> sample;
		sample.push_back(vec2i(0, 0));
		sample.push_back(vec2i(size.w / 2, 0));
		sample.push_back(vec2i(size.w / 2 + 10, 10));
		sample.push_back(vec2i(size.w / 2 - 10, 20));
		sample.push_back(vec2i(size.w / 2 + 10, 30));
		sample.push_back(vec2i(size.w / 2 - 10, 40));
		sample.push_back(vec2i(size.w / 2 + 10, 50));
		sample.push_back(vec2i(size.w / 2 - 10, 60));
		sample.push_back(vec2i(size.w / 2 + 10, 70));
		sample.push_back(vec2i(size.w / 2 - 10, 80));
		sample.push_back(vec2i(size.w / 2 + 10, 90));
		sample.push_back(vec2i(size.w, size.h));
		sample.push_back(vec2i(0, size.h));
		return sample;
	}
}

