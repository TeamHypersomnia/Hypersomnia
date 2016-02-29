#include "image.h"
#include "log.h"

namespace augs {
	std::vector<vec2i> image::get_polygonized() const {
		std::vector<vec2i> sample;
		sample.push_back(vec2i(0, 0));
		sample.push_back(vec2i(2, 0));
		sample.push_back(vec2i(4, 0));
		sample.push_back(vec2i(6, 0));
		sample.push_back(vec2i(8, 0));
		sample.push_back(vec2i(10, 0));
		sample.push_back(vec2i(12, 0));
		sample.push_back(vec2i(14, 0));
		sample.push_back(vec2i(16, 0));
		sample.push_back(vec2i(18, 0));
		sample.push_back(vec2i(20, 0));
		sample.push_back(vec2i(22, 0));
		sample.push_back(vec2i(24, 0));
		sample.push_back(vec2i(26, 0));
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

